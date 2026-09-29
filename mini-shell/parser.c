/* #include directives */
#include <stdio.h>
#include <string.h>
#include "parser.h"


/* Function Definitions */
// parse command
void parse_command(char *args[], char *input){

    int index = 0;

    char *token = strtok(input, " ");

    while(token != NULL && index < 15){
        args[index++] = token;
        token = strtok(NULL, " ");
    }

    args[index] = NULL;
}

// parse redirection
void parse_redirection(char **args, char **redirection_file, int *redirection_type, bool *redirection_error){
    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], ">") == 0){
            if(args[i+1] == NULL){
                printf("redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            args[i] = NULL;
            *redirection_type = 1;
            break;

        } else if (strcmp(args[i], ">>") == 0){
            if(args[i+1] == NULL){
                printf("Redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            args[i] = NULL;
            *redirection_type = 2;
            break;

        } else if(strcmp(args[i], "<") == 0){
            if(args[i+1] == NULL){
                printf("redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            args[i] = NULL;
            *redirection_type = 3;
            break;
        }
    }
}

// parse pipe
void parse_pipe(char **args, int *pipe_index, char **left_args, char **right_args){
    int total_index = 0;

    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], "|") == 0){
            *pipe_index = i;
        }
        total_index++;
    }

    if(*pipe_index != -1){
        for(int i = 0; i < *pipe_index; i++){
            left_args[i] = args[i]; 
        }
        left_args[*pipe_index] = NULL;

        int right_index = 0;

        for(int i = *pipe_index + 1; i < total_index; i++){
            right_args[right_index++] = args[i];
        }
        right_args[right_index] = NULL;
    }
}



