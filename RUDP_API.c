// RUDP_API.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

// Define RUDP packet structure
typedef struct {
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
} RUDPPacket;

int rudp_socket() {
    // Create a UDP socket and return the socket descriptor
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


/*
* @brief A random data generator function based on srand() and rand().
* @param size The size of the data to generate (up to 2^32 bytes).
* @return A pointer to the buffer.
*/
char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
    return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
    return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}





int rudp_send(int socket, const void *data, size_t len, struct sockaddr *dest_addr, socklen_t addrlen) {
    // Send data using RUDP protocol
    RUDPPacket packet;
    packet.length = htons(len);
    // Calculate checksum (dummy checksum for simplicity)
    packet.checksum = 0;
    packet.flags = 0; // Add flags as needed

    // Send packet
    if (sendto(socket, &packet, sizeof(RUDPPacket), 0, dest_addr, addrlen) == -1) {
        perror("Error sending packet");
        return -1;
    }

    // Send actual data
    if (sendto(socket, data, len, 0, dest_addr, addrlen) == -1) {
        perror("Error sending data");
        return -1;
    }

    return len;
}

int rudp_receive(int socket, void *buffer, size_t len, struct sockaddr *src_addr, socklen_t *addrlen) {
    // Receive data using RUDP protocol
    RUDPPacket packet;

    // Receive packet
    if (recvfrom(socket, &packet, sizeof(RUDPPacket), 0, src_addr, addrlen) == -1) {
        perror("Error receiving packet");
        return -1;
    }

    // Validate checksum (dummy checksum for simplicity)
    if (packet.checksum != 0) {
        fprintf(stderr, "Checksum validation failed\n");
        return -1;
    }

    // Receive actual data
    return recvfrom(socket, buffer, len, 0, src_addr, addrlen);
}

int rudp_close(int socket) {
    // Close the socket
    if (close(socket) == -1) {
        perror("Error closing socket");
        return -1;
    }
    return 0;
}
