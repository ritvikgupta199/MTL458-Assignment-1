#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

const int INIT_STR_SIZE = 100;
// const int INIT_CMD_SIZE = 20;
const int INIT_TOKEN_LEN = 10;

char* get_input();
char** get_tokens(char* input_str);
void run_command(char** tokens);
void update_curr_dir();
void change_dir(char** tokens);

char cwd[PATH_MAX];


int main() {
    update_curr_dir();
    while(1){
        printf("%s$ ", cwd);
        char* input = get_input();
        if (input == NULL){
            continue;
        }
        char** cmd_tokens = get_tokens(input);
        if (cmd_tokens == NULL){
            continue;
        } else {
            if (strcmp(cmd_tokens[0], "ps_history") == 0){
                printf("Print process history\n");
            } else if (strcmp(cmd_tokens[0], "cmd_history") == 0){
                printf("Print command history\n");
            } else if (strcmp(cmd_tokens[0], "cd") == 0){
                change_dir(cmd_tokens);
            } else if (strcmp(cmd_tokens[0], "exit") == 0){
                exit(1);
            } else {
                run_command(cmd_tokens);
            }     
        }
        free(cmd_tokens);
        free(input);
    }
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
    pid_t pid = fork();
    if (pid < 0){
        perror("fork error");
        exit(1);
    } else if (pid == 0){
        execvp(cmd_tokens[0], cmd_tokens);
        // if exec returns then there was an error
        perror("exec error");
        exit(1);
    } else {
        wait(NULL); // wait for child to finish
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