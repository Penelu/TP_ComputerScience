#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// Function to resolve the server address
void resolve_server(const char *server) {
    struct addrinfo hints, *res;
    char ip_str[INET_ADDRSTRLEN];  // Buffer to store the resolved IP address

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

    // Convert the resolved address to a readable IP string
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);
    write(STDOUT_FILENO, "Server resolved to IP: ", 23);
    write(STDOUT_FILENO, ip_str, strlen(ip_str));
    write(STDOUT_FILENO, "\n", 1);

    // Free the addrinfo structure
    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: puttftp <server> <file>\n", 31);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];

    write(STDOUT_FILENO, "Resolving server for puttftp...\n", 32);
    resolve_server(server);

    write(STDOUT_FILENO, "File to be uploaded: ", 21);
    write(STDOUT_FILENO, file, strlen(file));
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
