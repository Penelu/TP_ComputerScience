#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 4096  // Increased buffer size to handle larger blocks
#define DEFAULT_BLOCKSIZE 512

void build_wrq_with_blocksize(char *buffer, const char *filename, const char *mode, int blocksize) {
    uint16_t opcode = htons(2);  // WRQ opcode
    size_t len = 0;

    memcpy(buffer + len, &opcode, sizeof(opcode));
    len += sizeof(opcode);
    strcpy(buffer + len, filename);
    len += strlen(filename) + 1;
    strcpy(buffer + len, mode);
    len += strlen(mode) + 1;

    // Add the Blocksize option
    strcpy(buffer + len, "blksize");
    len += strlen("blksize") + 1;
    snprintf(buffer + len, 10, "%d", blocksize);
    len += strlen(buffer + len) + 1;

    write(STDOUT_FILENO, "WRQ with Blocksize option built\n", 32);
}

void send_wrq_with_blocksize_and_data(const char *server, const char *filename, int blocksize) {
    int sock;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    FILE *file;
    ssize_t received, sent;
    uint16_t block = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(server, "1069", &hints, &res) != 0) {
        write(STDOUT_FILENO, "Error resolving server address\n", 31);
        exit(EXIT_FAILURE);
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        write(STDOUT_FILENO, "Error creating socket\n", 23);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    file = fopen(filename, "rb");
    if (!file) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    build_wrq_with_blocksize(buffer, filename, "octet", blocksize);

    sent = sendto(sock, buffer, strlen(filename) + strlen("octet") + strlen("blksize") + 10 + 4, 0, res->ai_addr, res->ai_addrlen);
    if (sent == -1) {
        write(STDOUT_FILENO, "Error sending WRQ\n", 19);
        fclose(file);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "WRQ sent to server\n", 20);

    // Print chosen blocksize
    char blocksize_msg[64];
    snprintf(blocksize_msg, sizeof(blocksize_msg), "Blocksize chosen for transfer: %d bytes\n", blocksize);
    write(STDOUT_FILENO, blocksize_msg, strlen(blocksize_msg));

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (1) {
        // Wait for ACK
        received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_len);
        if (received == -1) {
            write(STDOUT_FILENO, "Error receiving ACK\n", 21);
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
        size_t data_len = fread(buffer + 4, 1, blocksize, file);
        if (data_len == 0) {
            write(STDOUT_FILENO, "File transfer complete\n", 24);
            break;
        }

        // Build DATA packet
        uint16_t data_opcode = htons(3);
        uint16_t data_block = htons(block);
        memcpy(buffer, &data_opcode, sizeof(uint16_t));
        memcpy(buffer + 2, &data_block, sizeof(uint16_t));

        sent = sendto(sock, buffer, data_len + 4, 0, (struct sockaddr *)&sender_addr, sender_len);
        if (sent == -1) {
            write(STDOUT_FILENO, "Error sending DATA packet\n", 26);
            break;
        }
    }

    fclose(file);
    close(sock);
    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        write(STDOUT_FILENO, "Usage: puttftp <server> <file> <blocksize>\n", 43);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];
    int blocksize = atoi(argv[3]);

    if (blocksize < 8 || blocksize > 65464) {
        write(STDOUT_FILENO, "Error: Blocksize must be between 8 and 65464\n", 46);
        exit(EXIT_FAILURE);
    }

    send_wrq_with_blocksize_and_data(server, file, blocksize);

    return 0;
}
