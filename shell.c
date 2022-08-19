#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

const int INIT_STR_SIZE = 100;
const int INIT_CMD_SIZE = 20;
const int INIT_TOKEN_SIZE = 10;

char* get_input();
char** get_tokens(char* input_str);
void run_command(char** tokens);



int main() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        exit(1);
    }
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
            } else if (strcmp(cmd_tokens[0], "exit") == 0){
                exit(1);
            } else {
                run_command(cmd_tokens);
            }     
        }
    }
}

void run_command(char** cmd_tokens){
    pid_t pid = fork();
    if (pid < 0){
        exit(1);
    } else if (pid == 0){
        execvp(cmd_tokens[0], cmd_tokens);
        exit(1);
    } else {
        wait(NULL);
    }
}


char* get_input(){
    char ch;
    int i = 0, s_size = INIT_STR_SIZE;
    char* s = malloc(s_size * sizeof(char));
    while((ch = getc(stdin)) != '\n'){
        if (i == s_size - 1){
            s_size += INIT_STR_SIZE;
            s = realloc(s, s_size * sizeof(char));
        }
        s[i++] = ch;
    }
    s[i] = '\0';
    return s;
}

char** get_tokens(char* input_str){
    int l = strlen(input_str);
    int t_idx = 0, cmd_idx = 0, t_size = INIT_TOKEN_SIZE, cmd_size = INIT_CMD_SIZE;
    char** tokens = malloc(t_size * sizeof(char*));
    bool esc = 0;

    for (int i = 0; i < l; i++){
        if (t_idx == t_size - 1){
            t_size += INIT_TOKEN_SIZE;
            tokens = realloc(tokens, t_size * sizeof(char*));
        }
        if (tokens[t_idx] == NULL){
            cmd_size = INIT_CMD_SIZE;
            tokens[t_idx] = malloc(cmd_idx * sizeof(char));
            cmd_idx = 0;
        }
        if (cmd_idx == cmd_size - 1){
            cmd_size += INIT_CMD_SIZE;
            tokens[t_idx] = realloc(tokens[t_idx], cmd_size * sizeof(char));
            printf("%s\n", tokens[t_idx]);
        }
        if (!esc && input_str[i] == ' '){
            if (cmd_idx == 0){
                continue;
            }
            tokens[t_idx++][cmd_idx+1] = '\0';
        } else{
            if (input_str[i] == '"'){
                esc = !esc;
            } else{
                tokens[t_idx][cmd_idx++] = input_str[i];
            }
            if (i == l-1){
                if (esc && input_str[i] != '"'){
                    return NULL;
                }
                tokens[t_idx++][cmd_idx+1] = '\0';
            }
        }
    }
    // for (int i = 0; i < t_idx; i++){
    //     printf("%s\n", tokens[i]);
    // }
    // To mark the end of the array
    tokens[t_idx] = NULL;
    return tokens;
}