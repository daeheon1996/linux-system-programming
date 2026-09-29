/* #include directives */
#include "executor.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>


/* Function Definitions */
// execute single command
void execute_single_command(char **args, char *redirection_file, int redirection_type){
    pid_t pid = fork();
    int status;

    if(pid < 0){
        perror("fork");
        return;
    }

    if(pid == 0){
        // child process
        if(redirection_file != NULL){

            if(redirection_type == 1){
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
                close(fd);

            } else if(redirection_type == 2){
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

        execvp(args[0], args);

        // execvp() returns only if command execution fails
        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        // parent process
        if(waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            return;
        }
        // display exit status 
        if(WIFEXITED(status)){
            printf("Process exited with status %d\n", WEXITSTATUS(status));
        }
    }
}

// execute_pipeline
void execute_pipeline(char **left_args, char **right_args){
    int pipefd[2];
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
        close(pipefd[0]);
        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("dup2");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        execvp(left_args[0], left_args);

        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if(pid2 < 0){
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        
        if(waitpid(pid1, NULL, 0) == -1){
            perror("waitpid");
            return;
        }

        return;

    } else if(pid2 == 0){
        close(pipefd[1]);
        if(dup2(pipefd[0], STDIN_FILENO) == -1){
            perror("dup2");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        close(pipefd[0]);

        execvp(right_args[0], right_args);

        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        // parent process
        close(pipefd[0]);
        close(pipefd[1]);

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
