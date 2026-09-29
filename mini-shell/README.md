## Overview

Mini Shell is a simple shell program implemented in C.
This project was built to practice core Linux system programming concepts such as process creation, command execution, I/O redirection, and pipes.

## Features

- Execute external commands
- Built-in commands: `cd`, `pwd`, `exit`
- I/O redirection: `<`, `>`, `>>`
- Single pipeline: `|`
- Error handling for system call failures

## Architecture

```text
User Input
    │
    ▼
  Parser
    │
    ▼
Built-in Command? ── Yes ──> Execute in Shell
    │ No
    ▼
Redirection / Pipe Check
    │
    ▼
   fork()
    │
    ├── Parent ──> waitpid()
    │
    └── Child
          │
          ├── Redirection ──> dup2()
          ├── Pipeline ─────> pipe() / dup2()
          │
          ▼
       execvp()
```

## Core Implementation

### 1. Process Execution

External commands are executed in a child process created with `fork()`.
The parent shell waits for the child process to finish using `waitpid()`.

```text
fork()
  │
  ├── Child  → execvp() → Execute command
  │
  └── Parent → waitpid() → Wait for child
```

`execvp()` replaces the child process image with the requested program while the parent shell remains running.

### 2. I/O Redirection

The shell supports input (`<`), output (`>`), and append (`>>`) redirection.

```text
open()
  ↓
dup2()
  ↓
Replace STDIN / STDOUT
  ↓
execvp()
```

The target file is opened with `open()`, and `dup2()` redirects standard input or standard output to the corresponding file descriptor before executing the command.

Examples:

```text
cat < input.txt
echo hello > output.txt
echo hello >> output.txt
```

### 3. Pipe

The shell supports a single pipeline connecting two commands.

Example:

```text
ls -l | grep .c
```

```text
Child 1                         Child 2
ls -l                           grep .c
STDOUT                          STDIN
   │                              ▲
   └────────── pipe ──────────────┘
```

A pipe is created with `pipe()`, and two child processes are created for the commands on each side of `|`.

```text
pipe()
  ↓
fork() Child 1 ──→ STDOUT → pipe write end
  ↓
fork() Child 2 ──→ STDIN  ← pipe read end
  ↓
close unused pipe file descriptors
  ↓
execvp()
```

`dup2()` connects the first command's standard output to the pipe's write end and the second command's standard input to the pipe's read end. Unused pipe file descriptors are closed in both the parent and child processes.

## Error Handling

The shell handles errors from major system calls and reports failures using `perror()`.

- `fork()` failure
- `execvp()` failure for invalid or unavailable commands
- `open()` failure for invalid or inaccessible files
- `pipe()` failure
- `dup2()` failure
- `chdir()` failure for invalid directories
- `getcwd()` failure
- `waitpid()` failure

When possible, errors are reported without terminating the main shell process, allowing the user to continue entering commands.

## Build & Run

Build the project using `make`:

```bash
make
```

Run the shell:

```bash
./miniShell
```

Clean generated object files and executable:

```bash
make clean
```

## Usage

### External Commands

```bash
myshell> ls -l
myshell> echo hello
```

### Built-in Commands

```bash
myshell> pwd
myshell> cd /tmp
myshell> exit
```

### I/O Redirection

```bash
myshell> echo hello > output.txt
myshell> echo world >> output.txt
myshell> cat < input.txt
```

### Pipe

```bash
myshell> ls -l | grep .c
```

## Limitations

- Supports only a single pipeline between two commands
- Does not support background execution (`&`)
- Does not support environment variable expansion
- Does not support command history or auto-completion
- Command parsing is limited compared to a real shell

## What I Learned

Through this project, I gained practical experience with core Linux system programming concepts:

- Process creation and execution using `fork()` and `execvp()`
- Parent-child process synchronization using `waitpid()`
- File descriptor manipulation using `open()` and `dup2()`
- I/O redirection for standard input and output
- Inter-process communication using `pipe()`
- Managing and closing file descriptors correctly
- Implementing built-in commands within the shell process
- Handling errors from Linux system calls