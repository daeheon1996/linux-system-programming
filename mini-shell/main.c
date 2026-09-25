// Project: Mini Shell

/* Stage 1: Basic Shell Loop

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

/* Stage 2: Built-in Commands

Requirements:
1. Implement the following built-in commands: exit, cd, pwd.
2. Built-in commands must be executed by the shell process itself.
3. Do not create a child process for bult-in commands.
4. The cd command changes the shell's current working directory.
5. The pwd command prints the shell's current working directory.
6. The exit command terminates the shell.
*/

/* Stage 3: Error Handling

Requirements:
1. Handle invalid commands without terminating the shell.
2. Handle missing arguments for the cd command.
3. Handle failures from getcwd() and chdir().
4. Handle fork() and waitpid() failures without terminating the shell.
5. Terminate only the child process if execvp() fails.
6. Handle EOF as normal shell termination.
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

        // read user input
        char input[256];
        if(fgets(input, sizeof(input), stdin) == NULL){
            if(feof(stdin)){
                printf("\nEnd of input\n");
                break;
            }

            perror("fgets");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';

        // handle empty input
        if(input[0] == '\0'){
            printf("Empty Input\n");
            continue;
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

        // handle built-in commands: exit, pwd, cd
        // 1. handle the "exit" built-in command
        if(strcmp(args[0], "exit") == 0){
            printf("Terminate Program\n");
            break;

        // 2. handle the "pwd" built-in command
        } else if(strcmp(args[0], "pwd") == 0) {
            char cwd[256];
            if(getcwd(cwd, sizeof(cwd)) != NULL){
                printf("%s\n", cwd);
            } else {
                perror("getcwd");
            }
            continue;

        // 3. handle the "cd" built-in command
        } else if(strcmp(args[0], "cd") == 0){ 
            if(args[1] == NULL){
                printf("cd : missing argument\n");
                continue;
            }

            if(chdir(args[1]) == -1){
                perror("cd");
            }
            continue;
        }

        // create a child process and handle exit status
        pid_t pid = fork();
        int status;

        if(pid < 0){
            perror("fork");
            continue;
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
                continue;
            }
            // display exit status 
            if(WIFEXITED(status)){
                printf("Process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }

    return 0;
}