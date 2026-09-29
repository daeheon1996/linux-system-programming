#ifndef PARSER_H
#define PARSER_H

/* #include directvies */
#include <stdbool.h>

/* Function Prototypes */
void parse_command(char *args[], char *input);
void parse_redirection(char **args, char **redirection_file, int *redirection_type, bool *redirection_error);
void parse_pipe(char **args, int *pipe_index, char **left_args, char **right_args);


#endif