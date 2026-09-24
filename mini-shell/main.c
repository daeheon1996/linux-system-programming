/*
Project: Mini Shell

Stage 1: Basic Shell Loop

Requirements:
1. Print "myshell> " when the shell starts.
2. Read a command line from user input.
   - Maximum input length: 256 characters
   - If the input is empty, prompt the user again without executing anything.
3. Parse the input into a command and its arguments.
   - Maximum number of arguments: 16
4. Create a new child process for each command.
5. Execute the command in the child process.
6. The parent process waits for the child process to terminate.
7. Repeat the process until the user enters "exit".
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

int main(void){
    while(true){
        printf("myshell> ");

        // get user input
        char input[256];
        if(fgets(input, sizeof(input), stdin) == NULL){
            return 1;
        }
        input[strcspn(input, "\n")] = '\0';

        // handle empty input
        if(input[0] == '\0'){
            printf("Empty Input\n");
            continue;
        }

        // if user input == "exit", terminate the program
        if(strcmp(input, "exit") == 0){
            printf("Terminate Program\n");
            break;
        }

        // parse user input 
        char *args[16];
        int index = 0;

        char *token = strtok(input, " ");
        while(token != NULL && index < 15){
            args[index++] = token;
            token = strtok(NULL, " ");
        }
        args[index] = NULL;

        // create a child process and handle exit status
        pid_t pid = fork();
        int status;

        if(pid < 0){
            perror("fork");
            return 1;
        }

        if(pid == 0){
            // child process
            execvp(args[0], args);

            // execvp() returns only if command execution fails
            perror("execvp");
            exit(EXIT_FAILURE);

        } else {
            // parent process
            if(waitpid(pid, &status, 0) == -1){
                perror("waitpid");
                return 1;
            }
            // display exit status 
            if(WIFEXITED(status)){
                printf("Process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }

    return 0;
}