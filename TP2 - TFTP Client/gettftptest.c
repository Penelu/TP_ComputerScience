#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 4096  // Buffer to handle larger blocks
#define DATA_OFFSET 4
#define BLOCKSIZE_TESTS 5  // Number of block sizes to test

// Function to measure transfer time for a specific blocksize
double measure_transfer_time(const char *server, const char *filename, int blocksize) {
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

    // Build RRQ packet with blocksize option
    uint16_t opcode = htons(1);  // RRQ opcode
    size_t len = 0;
    memcpy(buffer + len, &opcode, sizeof(opcode));
    len += sizeof(opcode);
    strcpy(buffer + len, filename);
    len += strlen(filename) + 1;
    strcpy(buffer + len, "octet");
    len += strlen("octet") + 1;
    strcpy(buffer + len, "blksize");
    len += strlen("blksize") + 1;
    snprintf(buffer + len, 10, "%d", blocksize);
    len += strlen(buffer + len) + 1;

    sent = sendto(sock, buffer, len, 0, res->ai_addr, res->ai_addrlen);
    if (sent == -1) {
        write(STDOUT_FILENO, "Error sending RRQ\n", 19);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    file = fopen("/dev/null", "wb");  // Discard data
    if (!file) {
        write(STDOUT_FILENO, "Error opening /dev/null\n", 24);
        close(sock);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    clock_t start = clock();  // Start timing

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
                fwrite(buffer + DATA_OFFSET, 1, received - DATA_OFFSET, file);

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
            write(STDOUT_FILENO, "Error received from server\n", 27);
            break;
        }

        if (received - DATA_OFFSET < blocksize) {
            break;
        }
    }

    fclose(file);
    close(sock);
    freeaddrinfo(res);

    clock_t end = clock();  // End timing
    return ((double)(end - start)) / CLOCKS_PER_SEC;  // Return elapsed time in seconds
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: gettftp <server> <file>\n", 31);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    const char *file = argv[2];

    int blocksize_tests[BLOCKSIZE_TESTS] = {512, 1024, 2048, 4096, 8192};
    double best_time = 1e9;
    int best_blocksize = 512;

    write(STDOUT_FILENO, "Testing different blocksizes...\n", 33);

    for (int i = 0; i < BLOCKSIZE_TESTS; i++) {
        int blocksize = blocksize_tests[i];
        double elapsed_time = measure_transfer_time(server, file, blocksize);
        char msg[128];
        snprintf(msg, sizeof(msg), "Blocksize %d: %.2f seconds\n", blocksize, elapsed_time);
        write(STDOUT_FILENO, msg, strlen(msg));

        if (elapsed_time < best_time) {
            best_time = elapsed_time;
            best_blocksize = blocksize;
        }
    }

    char result_msg[128];
    snprintf(result_msg, sizeof(result_msg), "Best blocksize: %d bytes (%.2f seconds)\n", best_blocksize, best_time);
    write(STDOUT_FILENO, result_msg, strlen(result_msg));

    return 0;
}
