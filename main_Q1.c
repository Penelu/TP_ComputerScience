#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char *argv[]){
    char* welcome = "Welcome to ENSEAshell!";
    char* welcome2 = "Type 'exit' to quit.";
    puts(welcome);
    puts(welcome2);

    while (1) {
        // Print the prompt
        write(STDOUT_FILENO, "enseash % ", 10);

        // Buffer to store user input
        char command[1024];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            // If user presses Ctrl+D, exit 
            puts("\nBye bye...");
            break;
        }

        // Remove trailing newline character from input
        command[strcspn(command, "\n")] = 0;

        // If user types 'exit', exit as well
        if (strcmp(command, "exit") == 0) {
            puts("Bye bye...");
            break;
        }

    }
    return 0;
}