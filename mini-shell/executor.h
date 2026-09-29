#ifndef EXECUTOR_H
#define EXECUTOR_H

/* Function Prototypes */
void execute_single_command(char **args, char *redirection_file, int redirection_type);
void execute_pipeline(char **left_args, char **right_args);

#endif