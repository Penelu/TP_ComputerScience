#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main() {
    // Welcome message
    const char *welcome_msg = "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n";
    write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));

    // REPL loop for prompt display
    while (1) {
        // Print the prompt
        const char *prompt = "enseash % ";
        write(STDOUT_FILENO, prompt, strlen(prompt));

        // Buffer to store user input
        char command[BUFFER_SIZE];
        ssize_t bytes_read = read(STDIN_FILENO, command, BUFFER_SIZE - 1);

    }

    return 0;
}