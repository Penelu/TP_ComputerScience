#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define MAX_BUFFER 1024
#define PROMPT_FORMAT "enseash %s%% "
#define WELCOME_MSG "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define GOODBYE_MSG "Bye bye...\n"
#define DEFAULT_PROMPT ""

// Calculate the elapsed time in milliseconds between two timespecs
long calculate_elapsed_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
}

// Parse the input command, separating arguments and handling redirections (< and >)
void parse_command(char *input, char *args[], char **input_file, char **output_file) {
    *input_file = NULL;
    *output_file = NULL;

    int i = 0;
    char *token = strtok(input, " ");
    while (token && i < MAX_BUFFER / 2 - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token) *input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token) *output_file = token;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

// Handle input and output redirections for the child process
void handle_redirections(char *input_file, char *output_file) {
    if (input_file) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror("Error opening input file");
            _exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("Error redirecting input");
            _exit(EXIT_FAILURE);
        }
        close(fd);
    }

    if (output_file) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("Error opening output file");
            _exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("Error redirecting output");
            _exit(EXIT_FAILURE);
        }
        close(fd);
    }
}

// Fork and execute the command with arguments and any necessary redirections
void execute_command(char *args[], char *input_file, char *output_file) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    } else if (pid == 0) {
        handle_redirections(input_file, output_file);
        execvp(args[0], args);
        perror("Command execution failed");
        _exit(EXIT_FAILURE);
    } else {
        waitpid(pid, NULL, 0);
    }
}

// Update the prompt status with the result of the last command (exit code or signal)
void update_prompt_status(int status, long elapsed_time, char *prompt_status) {
    if (WIFEXITED(status)) {
        snprintf(prompt_status, MAX_BUFFER, "[exit:%d|%ldms] ", WEXITSTATUS(status), elapsed_time);
    } else if (WIFSIGNALED(status)) {
        snprintf(prompt_status, MAX_BUFFER, "[sign:%d|%ldms] ", WTERMSIG(status), elapsed_time);
    } else {
        snprintf(prompt_status, MAX_BUFFER, "[unknown|%ldms] ", elapsed_time);
    }
}

// The main loop of the shell: read, parse, execute, and update prompt
void run_shell() {
    char prompt_status[MAX_BUFFER] = DEFAULT_PROMPT;
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));

    while (1) {
        char command[MAX_BUFFER], prompt[MAX_BUFFER];
        snprintf(prompt, MAX_BUFFER, PROMPT_FORMAT, prompt_status);
        write(STDOUT_FILENO, prompt, strlen(prompt));

        ssize_t bytes_read = read(STDIN_FILENO, command, MAX_BUFFER - 1);
        if (bytes_read <= 0) {
            write(STDOUT_FILENO, GOODBYE_MSG, strlen(GOODBYE_MSG));
            break;
        }

        command[bytes_read] = '\0';
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            write(STDOUT_FILENO, GOODBYE_MSG, strlen(GOODBYE_MSG));
            break;
        }

        if (strlen(command) == 0) continue;

        struct timespec start_time, end_time;
        if (clock_gettime(CLOCK_MONOTONIC, &start_time) < 0) {
            perror("Error getting start time");
            continue;
        }

        char *args[MAX_BUFFER / 2];
        char *input_file = NULL, *output_file = NULL;
        parse_command(command, args, &input_file, &output_file);
        execute_command(args, input_file, output_file);

        if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
            perror("Error getting end time");
            continue;
        }

        long elapsed_time = calculate_elapsed_time(start_time, end_time);
        int status;
        wait(&status);
        update_prompt_status(status, elapsed_time, prompt_status);
    }
}

int main() {
    run_shell();
    return 0;
}

