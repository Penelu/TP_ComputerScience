#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// Function to resolve the server address and reserve a connection socket
int create_socket(const char *server) {
    struct addrinfo hints, *res;
    int sock;

    // Configure hints for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP protocol
    hints.ai_protocol = IPPROTO_UDP;

    // Get address information
    if (getaddrinfo(server, "69", &hints, &res) != 0) {
        write(STDOUT_FILENO, "Error resolving server address\n", 31);
        exit(EXIT_FAILURE);
    }

    // Create the UDP socket
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        write(STDOUT_FILENO, "Error creating socket\n", 23);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "Socket created successfully\n", 28);

    // Free the addrinfo structure
    freeaddrinfo(res);

    return sock;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: gettftp <server> <file>\n", 31);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];

    write(STDOUT_FILENO, "Resolving server and reserving connection...\n", 45);
    int sock = create_socket(server);

    write(STDOUT_FILENO, "Server resolved successfully\n", 29);
    write(STDOUT_FILENO, "File to be downloaded: ", 23);
    write(STDOUT_FILENO, file, strlen(file));
    write(STDOUT_FILENO, "\n", 1);

    // Close the socket after use
    close(sock);
    write(STDOUT_FILENO, "Socket closed\n", 14);

    return 0;
}
