#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define INIT_STR_SIZE 128
#define INIT_TOKEN_LEN  32
#define HISTORY_SIZE 5
#define INIT_PID_LEN 32

const char* HIST_CMD = "cmd_history";
const char* PROC_HIST_CMD = "ps_history";
const char* CD_CMD = "cd";
const char* EXIT_CMD = "exit";
const char* PIPE_CHAR = "|";
const char BACKGROUND_CHAR = '&';

char cwd[PATH_MAX];

// Structure to store command history as a queue
struct History {
    char* cmd[HISTORY_SIZE];
    int front, rear;
};
struct History* cmd_history;

// Structure to store pid and status of a process
struct Process {
    pid_t pid;
    char* status;
};
// Structure to store process history
struct ProcessHistory {
    struct Process* procs;
    int size, capacity;
};
struct ProcessHistory ps_history;

void init();
void exit_handler();
void update_curr_dir();
char* get_input();
char** get_tokens(char* input_str);
int get_pipe(char** tokens);
bool is_env_var_assignment(char* cmd);
void set_env_var(char* env_cmd);
void replace_env_var(char** tokens);
void run_command(char** tokens);
void run_cmd_fork(char** tokens);
void run_cmd_pipe(char** tokens1, char** tokens2);
void run_cmd(char** tokens);
void change_dir(char** tokens);

struct History* create_queue();
void dequeue(struct History* queue);
void enqueue(struct History* queue, char* cmd);
void display_queue(struct History* queue);

void add_pid(pid_t pid, char* status);
void display_pids();
void update_status();
char* get_status(pid_t pid, bool is_background);


int main() {
    init(); // initialize signal handler, working directory, command history and process history
    while(1) {
        printf("%s~$ ", cwd); // print working directory and prompt
        char* input = get_input();
        if (input == NULL) {
            continue; // if input is NULL, continue to next iteration
        }
        char* cmd = strdup(input);
        char** cmd_tokens = get_tokens(input); // get command tokens from input
        if (cmd_tokens == NULL) {
            continue;
        } else {
            if (strcmp(cmd_tokens[0], CD_CMD) == 0) {
                change_dir(cmd_tokens); // change working directory
            } else if (strcmp(cmd_tokens[0], EXIT_CMD) == 0) {
                exit_handler(); // exit the shell
            } else {
                run_command(cmd_tokens); // run a command in a child process
            }
        }
        enqueue(cmd_history, cmd); // add the command to the command history
        free(cmd_tokens);
        free(input);
    }
}

// Initialize the shell
void init(){
    signal(SIGINT, exit_handler); // handle Ctrl+C signal to exit the shell
    update_curr_dir();
    cmd_history = create_queue(); // initiliaze command history queue
    ps_history.size = 0;
    ps_history.capacity = INIT_PID_LEN;
    ps_history.procs = (struct Process*)malloc(sizeof(struct Process) * ps_history.capacity);
}

// Exit the shell
void exit_handler(){
    exit(0);
}

// Create a new queue
struct History* create_queue(){
    struct History* queue = (struct History*)malloc(sizeof(struct History));
    queue->front = queue->rear = -1; // initially empty queue
    for (int i = 0; i < HISTORY_SIZE; i++) {
        queue->cmd[i] = NULL;
    }
    return queue;
}

// Dequeue a command from the queue
void dequeue(struct History* queue){
    if(queue->front == -1) {
        return; // queue is empty
    }
    free(queue->cmd[queue->front]); // free the memory
    if (queue->front == queue->rear) { // queue has only one element
        queue->front = queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % HISTORY_SIZE;
    }
    return;
}

// Enqueue a command in the queue
void enqueue(struct History* queue, char* cmd){
    if (queue->front == (queue->rear + 1) % HISTORY_SIZE) {
        // if the queue is full then dequeue the oldest element
        dequeue(queue);
    }
    if (queue->front == -1) { // queue is empty
        queue->front = 0;
    }
    queue->rear = (queue->rear + 1) % HISTORY_SIZE;
    queue->cmd[queue->rear] = cmd;
    return;
}

// Display the command history queue
void display_queue(struct History* queue){
    if (queue->front == -1) {
        printf("command history is empty\n"); // queue is empty
        return;
    } else if (queue->front == queue->rear) { // queue has only one element
        printf("%s\n", queue->cmd[queue->front]);
    } else {
        // display elements so that latest command appears first
        for (int i = queue->rear; i != queue->front; i = (i - 1 + HISTORY_SIZE) % HISTORY_SIZE) {
            printf("%s\n", queue->cmd[i]); // print all elements in the queue
        }
        printf("%s\n", queue->cmd[queue->front]); // print the remaining element at rear
    }
    return;
}

// Add pid to process history
void add_pid(pid_t pid, char* status){
    // if the size reaches capacity - 1, reallocate more space
    if (ps_history.size == ps_history.capacity - 1) {
        ps_history.capacity += INIT_PID_LEN;
        ps_history.procs = realloc(ps_history.procs, sizeof(struct Process) * ps_history.capacity);
        if (ps_history.procs == NULL) {
            perror("realloc error");
            exit(1);
        }
    }
    ps_history.procs[ps_history.size].pid = pid;
    ps_history.procs[ps_history.size++].status = status; // add process to the end of the array
}

// Display process history
void display_pids(){
    for (int i = 0; i < ps_history.size; i++) {
        printf("%d \t %s \n", ps_history.procs[i].pid, ps_history.procs[i].status);
    }
}

// Update the status of all background processes (those with RUNNING or NULL status)
void update_status(){
    for (int i = 0; i < ps_history.size; i++) {
        if (ps_history.procs[i].status == NULL || strcmp(ps_history.procs[i].status, "RUNNING") == 0) {
            ps_history.procs[i].status = get_status(ps_history.procs[i].pid, 1);
        }
    }
    return;
}

// Get status of a process with pid
char* get_status(pid_t pid, bool is_background){
    int status;
    // use WNOHANG and don't wait for background processes
    // use WUNTRACED and wait for foreground processes to finish
    pid_t flag = waitpid(pid, &status, is_background ? WNOHANG : WUNTRACED);
    if (flag < 0) {
        perror("waitpid error");
        return "WAITPID_ERROR";
    } else if (flag == 0) { // returns 0 if the process is still running
        return "RUNNING";
    } else { // returns pid of the stopped process
        return "STOPPED";
    }
    return NULL;
}

// Update the current working directory
void update_curr_dir(){
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        exit(1); // error in fetching working directory
    }
    return;
}

// Change working directory
void change_dir(char** cmd_tokens){
    if (cmd_tokens[1] == NULL || strcmp(cmd_tokens[1], "~") == 0) {
        // if no directory is specified or directory is ~, change to home directory
        cmd_tokens[1] = getenv("HOME");
    }
    if (chdir(cmd_tokens[1]) != 0) {
        perror("change directory failed");
    }
    update_curr_dir(); // update the current working directory
    return;
}

// Check if the command is for setting environment variable
bool is_env_var_assignment(char* cmd){
    for (int i = 0; i < strlen(cmd); i++) {
        if (cmd[i] == '='){
            return true;
        }
    }
    return false;
}

// Set environment variable
void set_env_var(char* env_cmd){
    char* var = strtok(env_cmd, "="); // name of the variable
    char* val = strtok(NULL, "="); // value of the variable
    if (var == NULL || val == NULL) { // name or value of variable is NULL
        perror("invalid command");
        return;
    } else {
        setenv(var, val, 1); // set the environment variable
    }
    return;
}

// Replace enviroment variables in commands with their values
void replace_env_var(char** cmd_tokens){
    for (int i = 0; cmd_tokens[i] != NULL; i++) {
        if (cmd_tokens[i][0] == '$') { // if the token is an environment variable
            char* env_var = cmd_tokens[i] + 1; // remove $ to get variable name
            char* env_val = getenv(env_var); // value of the environment variable
            if (env_val == NULL) { // if the environment variable is not set
                perror("environment variable not set");
                return;
            } else {
                cmd_tokens[i] = env_val; // replace the variable with its value
            }
        }
    }
    return;
}

// Run a command in a child process
void run_command(char** cmd_tokens){
    if (is_env_var_assignment(cmd_tokens[0])) { // if the command is for setting environment variable
        if (cmd_tokens[1] == NULL) { // if there is a single token
            set_env_var(cmd_tokens[0]);
            return;
        } else { // if there are one or more tokens after assignment
            perror("invalid command");
            return;
        }
    }
    if (strcmp(cmd_tokens[0], PROC_HIST_CMD) == 0) {
        update_status();
    }
    int pipe_loc = get_pipe(cmd_tokens); // check if the command is piped
    replace_env_var(cmd_tokens); // replace environment variables in the command
    if (pipe_loc < 0) { // command is not piped
        run_cmd_fork(cmd_tokens); // run a command without pipe
    } else {
        cmd_tokens[pipe_loc] = NULL; // split the command into two parts
        char** tokens1 = cmd_tokens;
        char** tokens2 = cmd_tokens + pipe_loc + 1;
        run_cmd_pipe(tokens1, tokens2);
    }
    return;
}

// Check if a command is piped and return the index of the pipe character
int get_pipe(char** cmd_tokens){
    for (int i = 0; cmd_tokens[i] != NULL; i++) {
        if (strcmp(cmd_tokens[i], PIPE_CHAR) == 0){
            return i; // return the location of the pipe
        }
    }
    return -1; // return -1 if no pipe is found
}

// Run a command with pipe
void run_cmd_pipe(char** tokens1, char** tokens2){
    int fd[2];
    pipe(fd); // create a pipe
    pid_t pid1 = fork(); // create first child process
    if (pid1 < 0) {
        perror("fork error");
        exit(1);
    } else if (pid1 == 0) { // first child process
        dup2(fd[WRITE_END], STDOUT_FILENO); // redirect output to pipe
        close(fd[READ_END]);
        close(fd[WRITE_END]);
        run_cmd(tokens1); // run the first command
    } else { // parent process
        pid_t pid2 = fork(); // create second child process
        if (pid2 < 0) {
            perror("fork error");
            exit(1);
        } else if (pid2 == 0) { // second child process
            dup2(fd[READ_END], STDIN_FILENO); // redirect input to pipe
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            run_cmd(tokens2); // run the second command
        } else { // parent process
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            // wait for the child processes to finish
            char* stat1 = get_status(pid1, 0);
            char* stat2 = get_status(pid2, 0);
            // add both process to history
            add_pid(pid1, stat1);
            add_pid(pid2, stat2);
        }
    }
    return;
}

// Run a command without pipe
void run_cmd_fork(char** cmd_tokens){
    bool is_background = cmd_tokens[0][0] == BACKGROUND_CHAR; // if the command is to be run in background
    cmd_tokens[0] = cmd_tokens[0][0] == BACKGROUND_CHAR ? cmd_tokens[0] + 1 : cmd_tokens[0]; // remove the '&' from the command
    pid_t pid = fork(); // create a child process
    if (pid < 0) {
        perror("fork error");
        exit(1);
    } else if (pid == 0) { // child process
        run_cmd(cmd_tokens); // run the command
    } else { // parent process
        char* status = get_status(pid, is_background); // wait for foreground process and get status for background process
        add_pid(pid, status); // add process to history
        if (is_background) {
            printf("[%d] %s\n", pid, cmd_tokens[0]); // print the pid and command of the background process
        }
    }
}

// Run command using exec or defined functions
void run_cmd(char** cmd_tokens){
    if (strcmp(cmd_tokens[0], PROC_HIST_CMD) == 0) {
        display_pids(); // display process history
    } else if (strcmp(cmd_tokens[0], HIST_CMD) == 0) {
        display_queue(cmd_history); // display command history
    } else {
        execvp(cmd_tokens[0], cmd_tokens); // run a command using exec
        perror("exec error"); // if exec returns then there was an error
        exit(1); // exit the child process
    }
    exit(0); // exit the child process
}

// Take arbitrary length input for the shell
char* get_input(){
    char ch;
    int i = 0, s_size = INIT_STR_SIZE;
    char* s = malloc(s_size * sizeof(char)); // allocate memory for string
    if (s == NULL) {
        perror("malloc error");
        exit(1);
    }
    while((ch = getc(stdin)) != '\n') {
        // if the index reaches the second last element of the string, reallocate more space
        // second last element is last non-null character
        if (i == s_size - 1) {
            s_size += INIT_STR_SIZE;
            s = realloc(s, s_size * sizeof(char));
            if (s == NULL){
                perror("realloc error");
                exit(1);
            }
        }
        s[i++] = ch;
    }
    if (i == 0) {
        return NULL; // if no input is entered, return null
    }
    s[i] = '\0'; // null terminate the string
    return s;
}

// Take input string and return array of tokens split by ' '
char** get_tokens(char* input_str){
    char** tokens = malloc(INIT_TOKEN_LEN * sizeof(char*)); // array of pointers to tokens
    int i = 0, t_size = INIT_TOKEN_LEN;
    char* p = strtok(input_str, " ");
    while(p != NULL) {
        // if the index reaches the second last element of the array, reallocate more space
        // second last element is the last non-NULL element of the array
        if (i == t_size - 1) {
            t_size += INIT_TOKEN_LEN;
            tokens = realloc(tokens, t_size * sizeof(char*));
            if (tokens == NULL) {
                perror("realloc error");
                exit(1);
            }
        }
        tokens[i++] = p;
        p = strtok(NULL, " ");
    }
    tokens[i] = NULL; // last element of the array is NULL
    return tokens;
}