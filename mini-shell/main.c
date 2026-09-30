/* #include Directives */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "executor.h"
#include "builtin.h"


/* #define Directives */
#define MAX_INPUT_SIZE 256
#define MAX_ARGS 16


/* Enum Definitions */
typedef enum {
    INPUT_OK,
    INPUT_RETRY,
    INPUT_EOF,
} InputStatus;


/* Function Definitions */
// read_user_input() -> Read and validate user input.
InputStatus read_user_input(char *input, size_t size){

    if(fgets(input, size, stdin) == NULL){
            if(feof(stdin)){
                printf("\nEnd of input\n");
                return INPUT_EOF;
            }

            perror("fgets");
            return INPUT_RETRY;
        }

        input[strcspn(input, "\n")] = '\0';

        // handle empty input
        if(input[0] == '\0'){
            printf("Empty Input\n");
            return INPUT_RETRY;
        }

    return INPUT_OK;
}


/* Main Definition */
int main(void){
    while(true){
        printf("myshell> ");

        // 1. read and validate user input
        char input[MAX_INPUT_SIZE];
        InputStatus input_status = read_user_input(input, sizeof(input));

        if(input_status == INPUT_RETRY){
            continue;
        } else if(input_status == INPUT_EOF){
            break;
        }

        // 2. parse command
        char *args[MAX_ARGS];
        parse_command(args, input);

        // 3. handle built-in commands: exit, pwd, cd
        BuiltinStatus builtin_status =  handle_builtin(args);

        if(builtin_status == BUILTIN_EXIT){
            break;
        } else if(builtin_status == BUILTIN_HANDLED){
            continue;
        }

        // 4. parse redirection: ">", ">>" or "<"
        char *redirection_file = NULL;
        int redirection_type = 0;
        bool redirection_error = false;
        parse_redirection(args, &redirection_file, &redirection_type, &redirection_error);

        if(redirection_error){
            continue;
        }

        // 5. parse pipe: "|"
        int pipe_index = -1;
        char *left_args[MAX_ARGS], *right_args[MAX_ARGS];
        parse_pipe(args, &pipe_index, left_args, right_args);
        
        
        // 6. execute single command (external)
        if(pipe_index == -1){
            execute_single_command(args, redirection_file, redirection_type);
        } 
        
        // 7. execute pipeline (two commands)
        else {
            execute_pipeline(left_args, right_args);
        }
    }

    return 0;
}

