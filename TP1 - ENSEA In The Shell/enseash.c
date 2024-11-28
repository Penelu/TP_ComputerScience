#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 1024

// Function to calculate elapsed time in milliseconds
long calculate_elapsed_time(struct timespec start, struct timespec end) {
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    return seconds * 1000 + nanoseconds / 1000000;  // Convert to milliseconds
}

int main(int argc, char *argv[]) {
    const char *welcome_msg = "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n";
    write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));

    // Variable to store the status of the last executed command
    char prompt_status[BUFFER_SIZE] = "";

    // REPL loop
    while (1) {
        // Construct the prompt dynamically
        char prompt[BUFFER_SIZE];
        snprintf(prompt, BUFFER_SIZE, "enseash %s%% ", prompt_status);
        write(STDOUT_FILENO, prompt, strlen(prompt));

        // Buffer to store user input
        char command[BUFFER_SIZE];
        ssize_t bytes_read = read(STDIN_FILENO, command, BUFFER_SIZE - 1);

        // Handle Ctrl+D (EOF) or read error
        if (bytes_read <= 0) {
            const char *goodbye_msg = "\nBye bye...\n";
            write(STDOUT_FILENO, goodbye_msg, strlen(goodbye_msg));
            break;
        }

        // Null-terminate the input string and remove trailing newline
        command[bytes_read] = '\0';
        command[strcspn(command, "\n")] = '\0';

        // If user types 'exit', break
        if (strcmp(command, "exit") == 0) {
            const char *goodbye_msg = "Bye bye...\n";
            write(STDOUT_FILENO, goodbye_msg, strlen(goodbye_msg));
            break;
        }

        // Ignore empty lines
        if (strlen(command) == 0) {
            continue;
        }

        // Measure execution time using clock_gettime
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        // Execute the command with a child fork
        pid_t pid = fork();
        if (pid < 0) {
            const char *error_msg = "Error: Fork failed\n";
            write(STDOUT_FILENO, error_msg, strlen(error_msg));
            continue;
        } else if (pid == 0) {
            // Child process: execute the command
            execlp(command, command, NULL);
            // If execlp fails, print an error message and exit
            const char *exec_fail_msg = "Command execution failed\n";
            write(STDERR_FILENO, exec_fail_msg, strlen(exec_fail_msg));
            exit(EXIT_FAILURE);
        } else {
            // Parent process: wait for the child to finish
            int status;
            waitpid(pid, &status, 0);

            // Measure the end time
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            // Calculate elapsed time
            long elapsed_time = calculate_elapsed_time(start_time, end_time);

            // Determine the status of the command
            if (WIFEXITED(status)) {
                // If the command exited normally, get the exit code
                int exit_code = WEXITSTATUS(status);
                snprintf(prompt_status, BUFFER_SIZE, "[exit:%d|%ldms] ", exit_code, elapsed_time);
            } else if (WIFSIGNALED(status)) {
                // If the command was terminated by a signal, get the signal number
                int signal_number = WTERMSIG(status);
                snprintf(prompt_status, BUFFER_SIZE, "[sign:%d|%ldms] ", signal_number, elapsed_time);
            } else {
                // Unknown status
                snprintf(prompt_status, BUFFER_SIZE, "[unknown|%ldms] ", elapsed_time);
            }
        }
    }

    return 0;
}
