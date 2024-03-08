#include "RUDP_API.c"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TIMEOUT 2 // Timeout value in seconds

// Structure to store statistics for each run
struct RunStatistics {
    int run_number;
    double elapsed_time_ms;
    double bandwidth;
    size_t total_bytes_received;
};

// Linked list node for statistics
struct StatisticsNode {
    struct RunStatistics stats;
    struct StatisticsNode *next;
};

// Function to print statistics
void print_statistics(const struct RunStatistics *stats) {
    printf("- Run #%d Data: Time=%.2fms; Speed=%.2fMB/s\n", stats->run_number, stats->elapsed_time_ms, stats->bandwidth / (1024 * 1024));
}

// Function to add a new node to the statistics linked list
struct StatisticsNode *add_statistics_node(struct StatisticsNode *head, const struct RunStatistics *stats) {
    struct StatisticsNode *new_node = (struct StatisticsNode *)malloc(sizeof(struct StatisticsNode));
    if (!new_node) {
        perror("Error allocating memory for statistics node");
        exit(EXIT_FAILURE);
    }

    new_node->stats = *stats;
    new_node->next = head;

    return new_node;
}

// Function to print all statistics in the linked list
void print_all_statistics(const struct StatisticsNode *head) {
    printf("* Statistics * -\n");
    while (head) {
        print_statistics(&head->stats);
        head = head->next;
    }
    printf("----------------------------------\n");
}

// Function to free the statistics linked list
void free_statistics_list(struct StatisticsNode *head) {
    while (head) {
        struct StatisticsNode *temp = head;
        head = head->next;
        free(temp);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Create UDP socket
    int sockfd = rudp_socket();

    struct sockaddr_in receiver_addr, sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    printf("Starting Receiver...\nWaiting for RUDP connection...\n");

    struct timeval start_time, end_time;
    size_t total_bytes_received = 0;
    int run_count = 0;
    int resend_count = 0;

    // Linked list to store statistics from sender
    struct StatisticsNode *statistics_list = NULL;

    while (1) {
        
        // Receive data
        char buffer[1024]; // Adjust the buffer size as needed
        ssize_t received_len = rudp_receive(sockfd, buffer, sizeof(buffer), (struct sockaddr *)&sender_addr, &sender_addr_len);

        if (received_len == -1) {
            fprintf(stderr, "Error receiving data\n");
            continue;
        }

        // Record the end time
        gettimeofday(&end_time, NULL);

        // Calculate the elapsed time in milliseconds
        double elapsed_time_ms = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                                 (end_time.tv_usec - start_time.tv_usec) / 1000.0;

        // Calculate the average bandwidth (bytes per second)
        double bandwidth = (double)total_bytes_received / (elapsed_time_ms / 1000.0);

        // Print information
        printf("File transfer completed.\nACK sent.\nWaiting for Sender response...\n");

        // Send acknowledgment (dummy acknowledgment for simplicity)
        char ack[] = "ACK";
        rudp_send(sockfd, ack, sizeof(ack), (struct sockaddr *)&sender_addr, sender_addr_len);

        // Check for exit message
        if (strncmp(buffer, "EXIT", 4) == 0) {
            gettimeofday(&start_time, NULL); // Initialize start_time before receiving any data

            // Print statistics for the last run
            struct RunStatistics current_stats = {++run_count, elapsed_time_ms, bandwidth, total_bytes_received};
            // print_statistics(&current_stats);

            // Add statistics to the linked list
            statistics_list = add_statistics_node(statistics_list, &current_stats);

            printf("Receiver end.\n");

            // Print all statistics
            print_all_statistics(statistics_list);

            // Free the linked list
            free_statistics_list(statistics_list);

            // Calculate the total average bandwidth
            double total_bandwidth = (double)total_bytes_received / ((end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                                                                      (end_time.tv_usec - start_time.tv_usec) / 1000.0);

            printf("- Average time: %.2fms\n", elapsed_time_ms / run_count);
            printf("- Total average bandwidth: %.2fMB/s\n", bandwidth / (1024 * 1024));
            printf("----------------------------------\n");
            break;
        }

        // Update total received bytes
        total_bytes_received += received_len;

        // Record the start time for the next run
        gettimeofday(&start_time, NULL);

        // Check if the sender wants to resend the file
        if (strncmp(buffer, "y", 1) == 0) {
            resend_count++;

            // Print statistics for each resend
            struct RunStatistics current_stats = {resend_count, elapsed_time_ms, bandwidth, total_bytes_received};
            // print_statistics(&current_stats);
             printf("- Run #%d Data: Time=%.2fms; Speed=%.2fMB/s\n", resend_count, elapsed_time_ms, bandwidth / (1024 * 1024));

            // Add statistics to the linked list
            // statistics_list = add_statistics_node(statistics_list, &current_stats);

            // Send acknowledgment for the resend request
            char resend_ack[] = "ACK";
            rudp_send(sockfd, resend_ack, sizeof(resend_ack), (struct sockaddr *)&sender_addr, sender_addr_len);
        }
    }

    // Close the socket
    rudp_close(sockfd);

    return 0;
}
