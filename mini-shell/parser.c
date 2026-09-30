/* #include directives */
#include <stdio.h>
#include <string.h>
#include "parser.h"


/* #define Directives */
#define MAX_ARGS 16


/* Function Definitions */
// parse_command() -> Parse user input into command arguments.
void parse_command(char *args[], char *input){
    int index = 0;

    char *token = strtok(input, " ");

    while(token != NULL && index < MAX_ARGS - 1){
        args[index++] = token;
        token = strtok(NULL, " ");
    }

    args[index] = NULL;
}

// parse_redirection() -> Parse I/O redirection operators: ">", ">>" or "<".
void parse_redirection(char **args, char **redirection_file, int *redirection_type, bool *redirection_error){

    for(int i = 0; args[i] != NULL; i++){
        // A redirection operator must be followed by a target file.
        if(strcmp(args[i], ">") == 0){
            if(args[i+1] == NULL){
                printf("redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            *redirection_type = 1;

            // Exclude the redirection operator and target file from execvp()
            args[i] = NULL;

            break;

        } else if (strcmp(args[i], ">>") == 0){
            // A redirection operator must be followed by a target file.
            if(args[i+1] == NULL){
                printf("Redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            *redirection_type = 2;

            // Exclude the redirection operator and target file from execvp()
            args[i] = NULL;

            break;

        } else if(strcmp(args[i], "<") == 0){
            // A redirection operator must be followed by a target file.
            if(args[i+1] == NULL){
                printf("redirection target missing\n");
                *redirection_error = true;
                break;
            }

            *redirection_file = args[i+1];
            *redirection_type = 3;

            // Exclude the redirection operator and target file from execvp()
            args[i] = NULL;
        
            break;
        }
    }
}

// parse_pipe() -> Parse the pipe operator "|" and seperate commands
void parse_pipe(char **args, int *pipe_index, char **left_args, char **right_args){
    int total_index = 0;

    // Find the pipe position and count the total number of arguments.
    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], "|") == 0){
            *pipe_index = i;
        }
        total_index++;
    }

    if(*pipe_index != -1){

        // Copy arguments before "|" to the left command.
        for(int i = 0; i < *pipe_index; i++){
            left_args[i] = args[i]; 
        }
        left_args[*pipe_index] = NULL;

        int right_index = 0;
        
        // Copy arguments after "|" to the right command.
        for(int i = *pipe_index + 1; i < total_index; i++){
            right_args[right_index++] = args[i];
        }
        right_args[right_index] = NULL;
    }
}



