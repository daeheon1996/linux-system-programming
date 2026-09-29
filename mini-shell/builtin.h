#ifndef BUILTIN_H
#define BUILTIN_H

/* Enum Definitions */
typedef enum {
    BUILTIN_HANDLED, // if the user input is a builtin funnction
    BUILTIN_NOT_FOUND, // uf the user input is NOT a builtin function
    BUILTIN_EXIT
} BuiltinStatus;


/* Function Prototypes */
BuiltinStatus handle_builtin(char **args);


#endif