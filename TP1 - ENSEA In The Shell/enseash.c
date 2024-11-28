#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
    const char *welcome_msg = "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n";
    write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));

    // Variable to store the status of the last executed command
    char prompt_status[BUFFER_SIZE] = "";

    // REPL loop
    while (1) {
        // Print the shell prompt with the last status
        char prompt[BUFFER_SIZE];
        int prompt_length = snprintf(prompt, BUFFER_SIZE, "enseash %s%% ", prompt_status);
        write(STDOUT_FILENO, prompt, prompt_length);

        // Adds buffer and reads input
        char command[BUFFER_SIZE];
        ssize_t bytes_read = read(STDIN_FILENO, command, BUFFER_SIZE - 1);

        // If Ctrl+D is pressed or read error, break
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

            // Determine the status of the command
            if (WIFEXITED(status)) {
                // If the command exited normally, get the exit code
                int exit_code = WEXITSTATUS(status);
                snprintf(prompt_status, BUFFER_SIZE, "[exit:%d] ", exit_code);
            } else if (WIFSIGNALED(status)) {
                // If the command was terminated by a signal, get the signal number
                int signal_number = WTERMSIG(status);
                snprintf(prompt_status, BUFFER_SIZE, "[sign:%d] ", signal_number);
            } else {
                // Unknown status
                strncpy(prompt_status, "[unknown] ", BUFFER_SIZE);
            }
        }
    }

    return 0;
}