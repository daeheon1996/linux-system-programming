/* #include directives */
#include "executor.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>


/* Function Definitions */
// execute_single_command() -> Execute a single external command in a child process.
void execute_single_command(char **args, char *redirection_file, int redirection_type){
    pid_t pid = fork();
    int status;

    if(pid < 0){
        perror("fork");
        return;
    }

    if(pid == 0){
        // Child process: set up I/O redirection before executing the command.
        if(redirection_file != NULL){
            
            if(redirection_type == 1){
                // Redirect stdout to a file and overwrite existing contents: ">".
                int fd = open(redirection_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if(fd == -1){
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if(dup2(fd, STDOUT_FILENO) == -1){
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                // fd is no longer needed after dup2().
                close(fd);

            } else if(redirection_type == 2){
                // Redirect stdout to a file and append to existing contents: ">>".
                int fd = open(redirection_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if(fd == -1){
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if(dup2(fd, STDOUT_FILENO) == -1){
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd);

            } else if(redirection_type == 3){
                // Redirect stdin to read input from a file: "<".
                int fd = open(redirection_file, O_RDONLY);
                if(fd == -1){
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if(dup2(fd, STDIN_FILENO) == -1){
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd);
            }
        }
        // Replace the child process with the requested external command.
        execvp(args[0], args);

        // execvp() returns only if command execution fails.
        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        // Parent process: wait for the child process to terminate.
        if(waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            return;
        }
        // Display the child's exit process if it terminated normally.
        if(WIFEXITED(status)){
            printf("Process exited with status %d\n", WEXITSTATUS(status));
        }
    }
}

// execute_pipeline() -> Execute two commands connected by a pipe.
void execute_pipeline(char **left_args, char **right_args){
    int pipefd[2];

    // Create a pipe: pipefd[0] for reading, pipefd[1] for writing.
    if(pipe(pipefd) == -1){
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if(pid1 < 0){
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;

    } else if(pid1 == 0){
        // Left child: send stdout to the pipe.
        close(pipefd[0]); // Left child does not read the pipe.

        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("dup2");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        // Original pipe fd is no longer needed after dup2().
        close(pipefd[1]);

        execvp(left_args[0], left_args);

        // Reached only if execvp() fails.
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if(pid2 < 0){
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        
        // Reap the already-created first child before returning.
        if(waitpid(pid1, NULL, 0) == -1){
            perror("waitpid");
            return;
        }

        return;

    } else if(pid2 == 0){
        // Right child: receive stdin from the pipe.
        close(pipefd[1]); // Right child does not write to the pipe.

        if(dup2(pipefd[0], STDIN_FILENO) == -1){
            perror("dup2");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        // Original pipe fd is no longer needed after dup2().
        close(pipefd[0]);

        execvp(right_args[0], right_args);

        // Reached only if execvp() fails.
        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        // Parent process does not use the pipe: closing both ends also allows EOF to be delivered when the writing child finishes.
        close(pipefd[0]);
        close(pipefd[1]);

        // Reap both child processes.
        if(waitpid(pid1, NULL, 0) == -1){
            perror("waitpid");
            return;
        }

        if(waitpid(pid2, NULL, 0) == -1){
            perror("waitpid");
            return;
        }
    }
}
