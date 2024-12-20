#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 516  // Maximum: 512 bytes of data + 4 bytes of header
#define DATA_SIZE 512    // Maximum data size in TFTP packet

// Function to build the WRQ packet
void build_wrq(char *buffer, const char *filename, const char *mode) {
    uint16_t opcode = htons(2);  // WRQ opcode
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

    write(STDOUT_FILENO, "WRQ packet built\n", 17);
}

// Function to send the WRQ and file data
void send_wrq(const char *server, const char *filename) {
    int sock;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    ssize_t sent, received;
    int file;
    uint16_t block = 0;

    // Resolve the server address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP protocol
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(server, "1069", &hints, &res) != 0) {
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

    // Open the file for reading
    file = open(filename, O_RDONLY);
    if (file == -1) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "Sending WRQ...\n", 16);
    // Build and send the WRQ packet
    build_wrq(buffer, filename, "octet");
    sent = sendto(sock, buffer, strlen(filename) + strlen("octet") + 4, 0, res->ai_addr, res->ai_addrlen);
    if (sent == -1) {
        write(STDOUT_FILENO, "Error sending WRQ\n", 19);
        close(file);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "WRQ sent to server\n", 20);

    // Send file data in DATA packets
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (1) {
        // Wait for the ACK from the server
        received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_len);
        if (received == -1) {
            write(STDOUT_FILENO, "Error receiving ACK\n", 21);
            perror("recvfrom");
            break;
        }

        uint16_t opcode, ack_block;
        memcpy(&opcode, buffer, sizeof(uint16_t));
        opcode = ntohs(opcode);

        if (opcode == 4) {  // ACK packet
            memcpy(&ack_block, buffer + 2, sizeof(uint16_t));
            ack_block = ntohs(ack_block);

            if (ack_block == block) {
                block++;
                char msg[128];
                snprintf(msg, sizeof(msg), "ACK received for block %d\n", ack_block);
                write(STDOUT_FILENO, msg, strlen(msg));
            }
        } else if (opcode == 5) {  // ERROR packet
            write(STDOUT_FILENO, "Error received from server: ", 28);
            write(STDOUT_FILENO, buffer + 4, strlen(buffer + 4));
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        // Read next chunk of data from the file
        ssize_t data_len = read(file, buffer + 4, DATA_SIZE);
        if (data_len == 0) {
            write(STDOUT_FILENO, "File transfer complete\n", 24);
            break;
        } else if (data_len == -1) {
            write(STDOUT_FILENO, "Error reading file\n", 20);
            break;
        }

        // Build the DATA packet
        uint16_t data_opcode = htons(3);
        uint16_t data_block = htons(block);
        memcpy(buffer, &data_opcode, sizeof(uint16_t));
        memcpy(buffer + 2, &data_block, sizeof(uint16_t));

        char debug_msg[128];
        snprintf(debug_msg, sizeof(debug_msg), "Sent DATA block %d, size %ld bytes\n", block, data_len);
        write(STDOUT_FILENO, debug_msg, strlen(debug_msg));

        // Send the DATA packet
        sent = sendto(sock, buffer, data_len + 4, 0, (struct sockaddr *)&sender_addr, sender_len);
        if (sent == -1) {
            write(STDOUT_FILENO, "Error sending DATA packet\n", 26);
            perror("sendto");
            break;
        }
    }

    // Clean up
    close(file);
    close(sock);
    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: puttftp <server> <file>\n", 31);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];

    send_wrq(server, file);
    return 0;
}
