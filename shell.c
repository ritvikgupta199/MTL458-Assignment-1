#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

const int INIT_STR_SIZE = 100;
const int INIT_TOKEN_LEN = 10;
const int HISTORY_SIZE = 5;
const char* HIST_CMD = "cmd_history";
const char* PROC_HIST_CMD = "ps_history";
const char* CD_CMD = "cd";
const char* EXIT_CMD = "exit";

// Structure to store command history as a queue
struct History {
    char* cmd[HISTORY_SIZE];
    int front, rear;
};
struct History* history_queue;

void update_curr_dir();
void init();
char* get_input();
char** get_tokens(char* input_str);
void run_command(char** tokens);
void change_dir(char** tokens);

struct History* create_queue();
void dequeue(struct History* queue);
void enqueue(struct History* queue, char* cmd);
void print_queue(struct History* queue);

char cwd[PATH_MAX];


int main() {
    init();
    while(1){
        printf("%s$ ", cwd);
        char* input = get_input();
        if (input == NULL) {
            continue;
        }
        char* cmd = strdup(input);
        char** cmd_tokens = get_tokens(input);
        if (cmd_tokens == NULL){
            continue;
        } else {
            if (strcmp(cmd_tokens[0], PROC_HIST_CMD) == 0){
                printf("Print process history\n");
            } else if (strcmp(cmd_tokens[0], HIST_CMD) == 0){
                print_queue(history_queue);
            } else if (strcmp(cmd_tokens[0], CD_CMD) == 0){
                change_dir(cmd_tokens);
            } else if (strcmp(cmd_tokens[0], EXIT_CMD) == 0){
                exit(0);
            } else {
                run_command(cmd_tokens);
            }     
        }
        enqueue(history_queue, cmd);
        free(cmd_tokens);
        free(input);
    }
}

// Initialize the shell
void init(){
    update_curr_dir();
    history_queue = create_queue();
}

// Create a new queue
struct History* create_queue(){
    struct History* queue = (struct History*)malloc(sizeof(struct History));
    queue->front = -1;
    queue->rear = -1;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        queue->cmd[i] = NULL;
    }
    return queue;
}

// Dequeue a command from the queue
void dequeue(struct History* queue){
    if(queue->front == -1){
        return; // queue is empty
    }
    queue->cmd[queue->front] = NULL;
    if (queue->front == queue->rear) { // queue has only one element
        queue->front = queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % HISTORY_SIZE;
    }
    return;
}

// Enqueue a command in the queue
void enqueue(struct History* queue, char* cmd){
    if (queue->front == (queue->rear + 1) % HISTORY_SIZE){
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

void print_queue(struct History* queue){
    if (queue->front == -1){
        printf("command history is empty\n"); // queue is empty
        return;
    } else if (queue->front == queue->rear){ // queue has only one element
        printf("%s\n", queue->cmd[queue->front]);
    } else {
        for (int i = queue->front; i != queue->rear; i = (i + 1) % HISTORY_SIZE){
            printf("%s\n", queue->cmd[i]); // print all elements in the queue
        }
        printf("%s\n", queue->cmd[queue->rear]); // print the remaining element at rear
    }
    return;
}

// Update the current working directory
void update_curr_dir(){
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        exit(1); // error in fetching working directory
    }
    return;
}

// Change working directory
void change_dir(char** cmd_tokens){
    if (cmd_tokens[1] == NULL){
        // if no directory is specified, change to home directory
        cmd_tokens[1] = getenv("HOME");
    }
    if (chdir(cmd_tokens[1]) != 0){
        perror("change directory failed");
    }
    update_curr_dir(); // update the current working directory
    return;
}

// Run commands using exec()
void run_command(char** cmd_tokens){
    bool is_background = false;
    if (cmd_tokens[0][0] == '&'){ // if the command is to be run in background
        is_background = true;
        cmd_tokens[0] = cmd_tokens[0] + 1; // set the str pointer to the next element
    }
    pid_t pid = fork();
    if (pid < 0){
        perror("fork error");
        exit(1);
    } else if (pid == 0){ // child process
        execvp(cmd_tokens[0], cmd_tokens);
        // if exec returns then there was an error
        perror("exec error");
        exit(1);
    } else { // parent process
        if (!is_background){ // if command is not background
            wait(NULL); // wait for child to finish
        } else {
            printf("[%d] %s\n", pid, cmd_tokens[0]); // print the pid and command of the background process
        }
    }
}

// Take arbitrary length input for the shell
char* get_input(){
    char ch;
    int i = 0, s_size = INIT_STR_SIZE;
    char* s = malloc(s_size * sizeof(char)); // allocate memory for string
    if (s == NULL){
        perror("malloc error");
        exit(1);
    }
    while((ch = getc(stdin)) != '\n'){
        // if the index reaches the second last element of the string, reallocate more space
        // second last element is last non-null character
        if (i == s_size - 1){
            s_size += INIT_STR_SIZE;
            s = realloc(s, s_size * sizeof(char));
            if (s == NULL){
                perror("realloc error");
                exit(1);
            }
        }
        s[i++] = ch;
    }
    if (i == 0){
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
            if (tokens == NULL){
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