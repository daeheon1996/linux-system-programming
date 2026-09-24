/*
Project: Mini Shell

Stage 1: Basic Shell Loop
Requirements:
1. After program execution, prints "myshell> "
2. Get a line of command from user input
- max length = 256
- empty input -> don't execute, get user input again
3. Parse the user input as command + arguments
- max num of arguments = 16
4. Everytime when executing command, create new child process
5. parent process must wait for the child process' execution
6. repeat the above process until user input == "exit"

*/