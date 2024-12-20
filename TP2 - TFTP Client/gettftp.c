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
#define DATA_SIZE 512    // Maximum data size in TFTP

void build_rrq(char *buffer, const char *filename, const char *mode) {
    uint16_t opcode = htons(1);  // RRQ opcode
    size_t len = 0;

    memcpy(buffer + len, &opcode, sizeof(opcode));
    len += sizeof(opcode);
    strcpy(buffer + len, filename);
    len += strlen(filename) + 1;
    strcpy(buffer + len, mode);
    len += strlen(mode) + 1;

    write(STDOUT_FILENO, "RRQ packet built\n", 17);
}

void send_rrq_and_receive(const char *server, const char *filename) {
    int sock;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    int file;
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

    build_rrq(buffer, filename, "octet");

    sent = sendto(sock, buffer, strlen(filename) + strlen("octet") + 4, 0, res->ai_addr, res->ai_addrlen);
    if (sent == -1) {
        write(STDOUT_FILENO, "Error sending RRQ\n", 19);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "RRQ sent to server\n", 20);

    file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        write(STDOUT_FILENO, "Error creating local file\n", 26);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (1) {
        received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_len);
        if (received == -1) {
            write(STDOUT_FILENO, "Error receiving data\n", 22);
            break;
        }

        uint16_t opcode, block_num;
        memcpy(&opcode, buffer, sizeof(uint16_t));
        opcode = ntohs(opcode);

        if (opcode == 3) {  // DATA packet
            memcpy(&block_num, buffer + 2, sizeof(uint16_t));
            block_num = ntohs(block_num);

            if (block_num == block + 1) {
                block++;
                char msg[128];
                snprintf(msg, sizeof(msg), "Received DATA block %d, size %ld bytes\n", block, received - 4);
                write(STDOUT_FILENO, msg, strlen(msg));

                if (write(file, buffer + 4, received - 4) == -1) {
                    write(STDOUT_FILENO, "Error writing to file\n", 22);
                    break;
                }

                // Send ACK
                uint16_t ack_opcode = htons(4);
                uint16_t ack_block = htons(block);
                memcpy(buffer, &ack_opcode, sizeof(uint16_t));
                memcpy(buffer + 2, &ack_block, sizeof(uint16_t));

                sent = sendto(sock, buffer, 4, 0, (struct sockaddr *)&sender_addr, sender_len);
                if (sent == -1) {
                    write(STDOUT_FILENO, "Error sending ACK\n", 18);
                    break;
                }
            }
        } else if (opcode == 5) {  // ERROR packet
            write(STDOUT_FILENO, "Error received from server: ", 28);
            write(STDOUT_FILENO, buffer + 4, strlen(buffer + 4));
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        if (received < BUFFER_SIZE) {
            write(STDOUT_FILENO, "File transfer complete\n", 24);
            break;
        }
    }

    close(file);
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

    send_rrq_and_receive(server, file);
    return 0;
}
