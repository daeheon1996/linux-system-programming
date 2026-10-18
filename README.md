# Linux System Programming Portfolio

A collection of projects developed in C to practice Linux system programming concepts, including process management, file I/O, inter-process communication, socket programming, and multithreading.

## Projects

### 1. Mini Shell

A simple Unix-like shell implemented in C.

**Key Features**
- Process creation and command execution using `fork()` and `execvp()`
- Built-in commands (`cd`, `pwd`, `exit`)
- Input/output redirection using `open()` and `dup2()`
- Command pipelines using `pipe()`
- Child process management using `waitpid()`

**[View Project](./mini-shell)**

### 2. Multithreaded HTTP Server

A lightweight HTTP/1.1 server implemented in C using POSIX threads.

**Key Features**
- TCP socket programming
- Concurrent client handling using `pthread`
- HTTP request parsing and response handling
- Thread synchronization using mutexes and condition variables
- Graceful shutdown using signal handling

**[View Project](./multithreaded-http-server)**

## Development Environment

- **Language:** C
- **Platform:** Linux
- **Build Tools:** GCC, Make
- **Version Control:** Git, GitHub
