#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function to display usage instructions
void print_usage(const char *program_name) {
    write(STDOUT_FILENO, "Usage: ", 7);
    write(STDOUT_FILENO, program_name, strlen(program_name));
    write(STDOUT_FILENO, " <server> <file>\n", 17);
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 3) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extract server and file from command-line arguments
    const char *server = argv[1];
    const char *file = argv[2];

    // Print the extracted information
    write(STDOUT_FILENO, "Server: ", 8);
    write(STDOUT_FILENO, server, strlen(server));
    write(STDOUT_FILENO, "\nFile: ", 7);
    write(STDOUT_FILENO, file, strlen(file));
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
