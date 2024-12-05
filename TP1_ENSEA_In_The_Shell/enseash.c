#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

// Function to calculate elapsed time in milliseconds
long calculate_elapsed_time(struct timespec start, struct timespec end) {
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    return seconds * 1000 + nanoseconds / 1000000;  // Convert to milliseconds
}

// Function to split a command into an array of arguments
void parse_command(char *command, char *args[], char **input_file, char **output_file) {
    int i = 0;
    char *token = strtok(command, " ");
    *input_file = NULL;
    *output_file = NULL;

    while (token != NULL && i < BUFFER_SIZE / 2 - 1) {
        if (strcmp(token, "<") == 0) {
            // Input redirection
            token = strtok(NULL, " ");
            if (token != NULL) {
                *input_file = token;
            }
        } else if (strcmp(token, ">") == 0) {
            // Output redirection
            token = strtok(NULL, " ");
            if (token != NULL) {
                *output_file = token;
            }
        } else {
            // Regular arguments
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Null-terminate the array
}

int main() {
    const char *welcome_msg = "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n";
    write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));

    char prompt_status[BUFFER_SIZE] = ""; // Last command status for prompt

    while (1) {
        // Construct the dynamic prompt
        char prompt[BUFFER_SIZE];
        snprintf(prompt, BUFFER_SIZE, "enseash %s%% ", prompt_status);
        write(STDOUT_FILENO, prompt, strlen(prompt));

        char command[BUFFER_SIZE];
        ssize_t bytes_read = read(STDIN_FILENO, command, BUFFER_SIZE - 1);

        // Handle Ctrl+D (EOF) or read error
        if (bytes_read <= 0) {
            write(STDOUT_FILENO, "\nBye bye...\n", 12);
            break;
        }

        // Null-terminate and trim newline
        command[bytes_read] = '\0';
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;
        }

        if (strlen(command) == 0) {
            continue;
        }

        // Measure start time
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        // Parse command
        char *args[BUFFER_SIZE / 2];
        char *input_file = NULL;
        char *output_file = NULL;
        parse_command(command, args, &input_file, &output_file);

        pid_t pid = fork();
        if (pid < 0) {
            write(STDOUT_FILENO, "Error: Fork failed\n", 19);
            continue;
        } else if (pid == 0) {
            // Handle input redirection
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("Error opening input file");
                    _exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Handle output redirection
            if (output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Error opening output file");
                    _exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Execute the command with arguments
            execvp(args[0], args);
            write(STDERR_FILENO, "Command execution failed\n", 25);
            _exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);

            // Measure end time
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            // Calculate elapsed time
            long elapsed_time = calculate_elapsed_time(start_time, end_time);

            // Update prompt status
            if (WIFEXITED(status)) {
                snprintf(prompt_status, BUFFER_SIZE, "[exit:%d|%ldms] ", WEXITSTATUS(status), elapsed_time);
            } else if (WIFSIGNALED(status)) {
                snprintf(prompt_status, BUFFER_SIZE, "[sign:%d|%ldms] ", WTERMSIG(status), elapsed_time);
            } else {
                snprintf(prompt_status, BUFFER_SIZE, "[unknown|%ldms] ", elapsed_time);
            }
        }
    }

    return 0;
}

