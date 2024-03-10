/************************************************************
* Name: ISLAM Nafis Ul
* UID: 3035835571
* Development platform: MacOS Sonoma ARM64 / Ubuntu 20.04.2 LTS ARM64 and x86_64
* Compilation command: gcc -o JCshell_3035835571 JCshell_3035835571.c -std=c11 -pedantic-errors -Wall -Werror
* Remarks: Completed the basic items without bonus features
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_INPUT_LENGTH 1025                    // Maximum length of user input
#define MAX_CHILDREN 5                          // Maximum number of child processes (can be changed)

// I'm blind so I'll define some colors to make the output easier to read
#define ANSI_COLOR_GREEN "\x1b[32m"             // Green
#define ANSI_COLOR_YELLOW "\x1b[38;5;185m"      // YELLOW
#define ANSI_COLOR_LIGHT_BLUE "\x1b[94m"        // Light blue
#define ANSI_COLOR_RED "\x1b[31m"               // Red
#define ANSI_COLOR_RESET "\x1b[0m"              // Reset to white


// Struct to store command, PID, and output of a child process
typedef struct child_process
{
    char * command;
    pid_t pid;
    int fd[2];
} child_process;


// Allocate memory for an array of child_process structs
struct child_process processes[MAX_CHILDREN];
int pfd[MAX_CHILDREN - 1];                      // Array of pipe file descriptors


// Function Constructors
void sigintHandler(int signal);                 // Signal handler for SIGINT
void create_child_process(char * input);       // Function to create a forked child process
int parse_input(char * input);                  // Function to create a child_process struct
int reset_process_array(void);                  // Function to reset the array of child_process structs
int print_processes(void);                      // Function to print the array of child_process structs
int get_num_processes(void);                    // Function to get the number of child processes
void reset_processes(void);                     // Function to reset the array of child_process structs
char* get_process_status(int pid);              // Function to get the status of a process

int main(void)
{
    char input[MAX_INPUT_LENGTH];
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        processes[i].pid = 0;
        processes[i].command = NULL;
        pipe(processes[i].fd);
    }

    // Register the signal handler for SIGINT
    signal(SIGINT, sigintHandler);

    // Loop forever
    while (1)
    {
        // Print shell prompt
        pid_t pid = getpid();
        printf("\n%s## JCshell [%s%d%s] %s##\t", ANSI_COLOR_GREEN, ANSI_COLOR_YELLOW, pid, ANSI_COLOR_GREEN, ANSI_COLOR_RESET);

        // Read user input
        if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL)
        {
            // If fgets encounters an error or end-of-file, exit the program
            printf("Goodbye!\n");
            exit(0);
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Check if the user entered the exit command
        if (strncmp(input, "exit", 4) == 0)
        {
            // Ensure that "exit" is the only input
            bool hasOtherArguments = (strlen(input) > 4 && input[4] != '\0');
            if (hasOtherArguments)
            {
                printf("JCshell: \"exit\" with other arguments!!!\n");
                continue;  // Skip the rest of the loop and start over
            }

            printf("JCshell: Terminated\n");
            exit(0);
        }

        parse_input(input);
        create_child_process(input);
    }
    return 0;
}

void create_child_process(char * input) {
    int i;
    int in = 0;
    int pipefd[2];

    int num_processes = get_num_processes();

    for (i = 0; i < num_processes; i++) {
        if (i < num_processes - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process

            // Set up input redirection
            if (i > 0) {
                dup2(in, STDIN_FILENO);
                close(in);
            }

            // Set up output redirection
            if (i < num_processes - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            // Parse the command and arguments
            char* command = strtok(processes[i].command, " ");
            char* arguments[100];
            arguments[0] = command;
            int arg_count = 1;

            char* arg = strtok(NULL, " ");
            while (arg != NULL) {
                arguments[arg_count++] = arg;
                arg = strtok(NULL, " ");
            }
            arguments[arg_count] = NULL;

            // Execute the command
            execvp(command, arguments);

            // If execvp fails, print an error
            perror("JCshell");
            exit(EXIT_FAILURE);
        } else {
            // Parent process

            // Close unused file descriptors
            if (i > 0) {
                close(in);
            }
            if (i < num_processes - 1) {
                close(pipefd[1]);
                in = pipefd[0];
            }
            
            processes[i].pid = pid;
        }
    }

    // Wait for each child process to finish
    for (i = 0; i < num_processes; i++) {
        waitpid(processes[i].pid, NULL, 0);
    }

    // Print the status of each child process
    for (i = 0; i < num_processes; i++) {
        char* status = get_process_status(processes[i].pid);
        printf("%s\n", status);
        free(status);
    }
    reset_process_array();


}



void sigintHandler(int signal)
{

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        if (processes[i].pid != 0)
        {
            // Send SIGUSR1 signal to child process to terminate it
            kill(processes[i].pid, SIGINT);
            break;
        }
        else{
            pid_t pid = getpid();
            printf("\n%s## JCshell [%s%d%s] %s##\t", ANSI_COLOR_GREEN, ANSI_COLOR_YELLOW, pid, ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
            fflush(stdout);
            break;
        }
    }

}

int parse_input(char* input) 
{
    char* token = strtok(input, "|");
    int max_processes = MAX_CHILDREN;
    while (token != NULL) 
    {
        if (max_processes == 0) 
        {
            printf("Error: Too many processes\n");
            // Empty the array of child_process structs
            reset_process_array();
            return -1;
        }
        // Strip leading and trailing spaces from command
        char* command = token;
        char* end = token + strlen(token) - 1;
        while (*command == ' ' && command <= end) 
        {
            command++;
        }
        while (*end == ' ' && end > command) 
        {
            *end = '\0';
            end--;
        }
        // Check if command is an empty string
        if (strlen(command) == 0) 
        {
            printf("JCShell: should not have two | symbols without in-between command\n");
            reset_process_array();
            return -1;
        }
        else
        {
            int process_index = MAX_CHILDREN - max_processes;
            processes[process_index].command = command;
            processes[process_index].pid = 0;
            max_processes--;
        }

        token = strtok(NULL, "|");
    }
    return 0;
}

int print_processes(void) 
{
    for (int i = 0; i < MAX_CHILDREN; i++) 
    {
        if (processes[i].pid == 0) 
        {
            printf("PID: %d\n", processes[i].pid);
            printf("Command: %s\n", processes[i].command);
        }
    }
    return 0;
}

int reset_process_array(void) 
{
    for (int i = 0; i < MAX_CHILDREN; i++) {
        processes[i].command = NULL;
        processes[i].pid = 0;
        processes[i].fd[0] = -1;
        processes[i].fd[1] = -1;
    }
    return 0;
}

int get_num_processes(void) 
{
    int num_processes = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) 
    {
        if (processes[i].command != NULL) 
        {
            num_processes++;
        }
    }
    return num_processes;
}

// Get the status of a process
char* get_process_status(int pid) {
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    char path[50];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open stat file for process %d\n", pid);
        return NULL;
    }

    // Read the stat file
    char stat_buf[1024];
    if (fgets(stat_buf, sizeof(stat_buf), file) == NULL) {
        fprintf(stderr, "Failed to read stat file for process %d\n", pid);
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Tokenize the stat file contents
    const char* delim = " ";
    char* token = strtok(stat_buf, delim);
    int token_count = 0;

    // Iterate over tokens until the desired information is found
    char* cmd = NULL;
    char* state = NULL;
    char* excode = NULL;
    char* ppid = NULL;
    char* user = NULL;
    char* sys = NULL;

    while (token != NULL && token_count < 24) {
        token_count++;

        switch (token_count) {
            case 2:
                cmd = strdup(token);  // Allocate memory for the command
                break;
            case 3:
                state = strdup(token);
                break;
            case 23:
                excode = strdup(token);
                break;
            case 4:
                ppid = strdup(token);
                break;
            case 14:
                user = strdup(token);
                break;
            case 15:
                sys = strdup(token);
                break;
        }

        token = strtok(NULL, delim);
    }

    // Construct the status string
    char* status = (char*)malloc(256 * sizeof(char));
    snprintf(status, 256, "(PID)%d (CMD)%s (STATE)%s (EXCODE)%s (PPID)%s (USER)%s (SYS)%s (VCTX)0 (NVCTX)0",
             pid, cmd, state, excode, ppid, user, sys);

    // Free allocated memory
    free(cmd);
    free(state);
    free(excode);
    free(ppid);
    free(user);
    free(sys);

    return status;
}
