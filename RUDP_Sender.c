#include "RUDP_API.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define TIMEOUT 2 // Timeout value in seconds
#define MAX_PACKET_SIZE 1400 // Adjust as needed
#define FILE_SIZE (2 * 1024 * 1024) // 2MB file size

void send_fragmented_data(int sockfd, const char *data, size_t data_size, struct sockaddr *dest_addr, socklen_t addrlen) {
    size_t offset = 0;

    while (offset < data_size) {
        size_t chunk_size = (data_size - offset > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : (data_size - offset);

        int attempts = 0;
        int sent_len = -1;

        while (attempts < 3) { // Retry up to 3 times
            sent_len = rudp_send(sockfd, data + offset, chunk_size, dest_addr, addrlen);

            if (sent_len != -1) {
                // Successful send
                break;
            }

            // Wait for a short time before retransmitting
            sleep(TIMEOUT);
            attempts++;
        }

        if (attempts == 3) {
            fprintf(stderr, "Failed to send data after 3 attempts. Exiting.\n");
            break;
        }

        offset += chunk_size;
    }
}

int main(int argc, char *argv[]) {
    char choice = 'Y';
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <receiver_ip> <receiver_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *receiver_ip = argv[1];
    int receiver_port = atoi(argv[2]);

    // Create UDP socket
    int sockfd = rudp_socket();

    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);

    // Convert receiver IP address to network format
    if (inet_pton(AF_INET, receiver_ip, &receiver_addr.sin_addr) <= 0) {
        perror("Error converting IP address");
        exit(EXIT_FAILURE);
    }

    // Send SYN message for connection request
    const char *syn_message = "SYN";
    rudp_send(sockfd, syn_message, strlen(syn_message), (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

    // Receive acknowledgment for connection request
    char ack_message[3];
    ssize_t ack_len = rudp_receive(sockfd, ack_message, sizeof(ack_message), NULL, NULL);

    if (ack_len == -1 || strncmp(ack_message, "ACK", 3) != 0) {
        fprintf(stderr, "Error receiving acknowledgment for connection request. Exiting.\n");
        rudp_close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connection established, beginning to send file...\n");

    // Generate random data for the file
    char *file_data = util_generate_random_data(FILE_SIZE);

    do {
        // Send the file using RUDP with fragmentation and timeout mechanism
        send_fragmented_data(sockfd, file_data, FILE_SIZE, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

        // Send exit message to initiate a resend if the choice is 'y'
        const char *exit_message = (choice == 'Y' || choice == 'y') ? "y" : "n";
        rudp_send(sockfd, exit_message, strlen(exit_message), (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

        // Receive acknowledgment for resend request
        char resend_ack[3];
        ssize_t resend_ack_len = rudp_receive(sockfd, resend_ack, sizeof(resend_ack), NULL, NULL);

        if (resend_ack_len == -1 || strncmp(resend_ack, "ACK", 3) != 0) {
            fprintf(stderr, "Error receiving acknowledgment for resend request. Exiting.\n");
            rudp_close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Check for statistics message
        char statistics_message[1024]; // Adjust the buffer size as needed
        ssize_t statistics_len = rudp_receive(sockfd, statistics_message, sizeof(statistics_message), NULL, NULL);

        if (statistics_len != -1 && strncmp(statistics_message, "STATS", 5) == 0) {
            // Process and print statistics
            printf("Statistics received from the receiver:\n%s", statistics_message);
        }

        // Ask for resending the file
        printf("Do you want to resend the file? (y/n): ");
        scanf(" %c", &choice);

    } while (choice == 'Y' || choice == 'y');

    // Send exit message
    const char *exit_message = "EXIT";
    rudp_send(sockfd, exit_message, strlen(exit_message), (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

    // Close the socket
    rudp_close(sockfd);

    // Free the allocated file_data memory
    free(file_data);

    return 0;
}
