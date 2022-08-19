#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const int INIT_STR_SIZE = 100;
const int CMD_SIZE = 20;
const int INIT_CMD_SIZE = 10;

char* get_input();
char** get_tokens(char* input_str);

int main() {
    while(1){
        char* input = get_input();
        if (input == NULL){
            printf("\n");
            continue;
        }
        printf("%s\n", input);
        // char** cmd_tokens = get_tokens(input);
    }
}

char* get_input(){
    char* s = malloc(INIT_STR_SIZE * sizeof(char));
    char ch;
    int i = 0, s_size = INIT_STR_SIZE;
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
    char** tokens = malloc(INIT_CMD_SIZE * sizeof(char*));
    int t_idx = 0, cmd_idx = 0, t_size = INIT_CMD_SIZE;
    bool esc = 0;

    for (int i = 0; i < l; i++){
        if (t_idx == t_size - 1){
            t_size += INIT_CMD_SIZE;
            tokens = realloc(tokens, t_size * sizeof(char*));
        }
        if (tokens[t_idx] == NULL){
            tokens[t_idx] = malloc(CMD_SIZE * sizeof(char));
            cmd_idx = 0;
        }
        if (!esc && input_str[i] == ' '){
            tokens[t_idx++][cmd_idx+1] = '\0';
        }
        else{
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
    printf("\n");
    for (int i = 0; i < t_idx; i++){
        printf("%s\n", tokens[i]);
    }
    return tokens;
}