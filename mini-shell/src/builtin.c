/* #include directives */
#include "builtin.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/* #define Directives */
#define MAX_INPUT_SIZE 256


/* Function Definitions */
// handle_builtin() -> Handle built-in commands: pwd, cd, exit.
BuiltinStatus handle_builtin(char **args){
    // 1. handle the "exit" built-in command
        if(strcmp(args[0], "exit") == 0){
            printf("Terminate Program\n");
            return BUILTIN_EXIT;

    // 2. handle the "pwd" built-in command
    } else if(strcmp(args[0], "pwd") == 0) {
        char cwd[MAX_INPUT_SIZE];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return BUILTIN_HANDLED;

    // 3. handle the "cd" built-in command
    } else if(strcmp(args[0], "cd") == 0){ 
        if(args[1] == NULL){
            printf("cd : missing argument\n");
            return BUILTIN_HANDLED;
        }

        if(chdir(args[1]) == -1){
            perror("cd");
        }
        return BUILTIN_HANDLED;
    }

    return BUILTIN_NOT_HANDLED;
}


