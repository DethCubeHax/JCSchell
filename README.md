# JCshell

JCshell is a simple shell program written in C that allows users to execute commands and manage child processes. It supports basic command execution, process management, and signal handling.

## Features

- Executes user-entered commands using `execvp()`
- Supports piping between commands using `|`
- Limits the maximum number of child processes to `MAX_CHILDREN` (default is 5)
- Handles the `SIGINT` signal to terminate child processes gracefully
- Provides process information such as PID, command, state, exit code, PPID, user time, and system time
- Colorful shell prompt with the current process ID

## Usage

1. Compile the program using the following command:
   ```
   gcc -o JCshell JCshell.c -std=c11 -pedantic-errors -Wall -Werror
   ```

2. Run the compiled executable:
   ```
   ./JCshell
   ```

3. Enter commands at the shell prompt. You can execute single commands or pipe multiple commands using `|`.

4. To exit the shell, enter the `exit` command.

## Code Structure

The code is organized into several functions:

- `main()`: The main function that runs the shell loop and handles user input.
- `create_child_process()`: Creates a child process for each command entered by the user and sets up input/output redirection using pipes.
- `parse_input()`: Parses the user input and creates a `child_process` struct for each command.
- `sigintHandler()`: Signal handler for `SIGINT` that terminates child processes gracefully.
- `print_processes()`: Prints information about the child processes.
- `reset_process_array()`: Resets the array of `child_process` structs.
- `get_num_processes()`: Returns the number of child processes.
- `get_process_status()`: Retrieves the status information of a process.

The program uses a `child_process` struct to store information about each child process, including the command, PID, and file descriptors for input/output redirection.

## Limitations

- The program supports a maximum of `MAX_CHILDREN` child processes at a time. This value can be adjusted by modifying the `MAX_CHILDREN` macro.
- The program does not support advanced shell features such as background processes, job control, or file redirection using `>` or `<`.
- The program assumes a Unix-like environment and uses system calls specific to Unix-based operating systems.


## Development Platform

- MacOS Sonoma ARM64
- Ubuntu 20.04.2 LTS ARM64 and x86_64

## Compilation Command

```
gcc -o JCshell JCshell.c -std=c11 -pedantic-errors -Wall -Werror
```

## Remarks

The program implements the basic functionality of a shell without additional bonus features.