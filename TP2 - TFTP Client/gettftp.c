#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 516  // Maximum: 512 bytes of data + 4 bytes of header

// Function to build the RRQ packet
void build_rrq(char *buffer, const char *filename, const char *mode) {
    uint16_t opcode = htons(1);  // RRQ has opcode 1
    size_t len = 0;

    // Add opcode
    memcpy(buffer + len, &opcode, sizeof(opcode));
    len += sizeof(opcode);

    // Add the filename
    strcpy(buffer + len, filename);
    len += strlen(filename) + 1;

    // Add the transfer mode
    strcpy(buffer + len, mode);
    len += strlen(mode) + 1;

    write(STDOUT_FILENO, "RRQ packet built\n", 17);
}

// Function to send the RRQ and receive the first response
void send_rrq(const char *server, const char *filename) {
    int sock;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    ssize_t received;

    // Resolve the server address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP protocol
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(server, "69", &hints, &res) != 0) {
        write(STDOUT_FILENO, "Error resolving server address\n", 31);
        exit(EXIT_FAILURE);
    }

    // Create the UDP socket
    if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        write(STDOUT_FILENO, "Error creating socket\n", 23);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Build the RRQ packet
    build_rrq(buffer, filename, "octet");

    // Send the RRQ packet
    if (sendto(sock, buffer, strlen(filename) + strlen("octet") + 4, 0, res->ai_addr, res->ai_addrlen) == -1) {
        write(STDOUT_FILENO, "Error sending RRQ\n", 19);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "RRQ sent to server\n", 20);

    // Prepare to receive the first DATA or ERROR packet
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_len);
    if (received == -1) {
        write(STDOUT_FILENO, "Error receiving server response\n", 33);
    } else {
        uint16_t opcode, block;
        memcpy(&opcode, buffer, sizeof(uint16_t));
        opcode = ntohs(opcode);

        if (opcode == 3) {  // DATA packet
            memcpy(&block, buffer + 2, sizeof(uint16_t));
            block = ntohs(block);
            char msg[128];
            snprintf(msg, sizeof(msg), "Received DATA packet, block %d, size %ld bytes\n", block, received - 4);
            write(STDOUT_FILENO, msg, strlen(msg));
        } else if (opcode == 5) {  // ERROR packet
            write(STDOUT_FILENO, "Error received from server: ", 28);
            write(STDOUT_FILENO, buffer + 4, strlen(buffer + 4));
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    // Clean up
    close(sock);
    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: gettftp <server> <file>\n", 31);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];

    write(STDOUT_FILENO, "Sending RRQ...\n", 16);
    send_rrq(server, file);

    return 0;
}
