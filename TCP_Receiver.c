#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <arpa/inet.h>

#define MAXCLIENTS 1
#define BUFFER_SIZE 65536

// LinkedList to store the time taken to receive the file and the file size each time
typedef struct Node {
    size_t fileSize;
    double timeTaken;
    struct Node *next;
} Node;

//new node function
Node *newNode(size_t fileSize, double timeTaken) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->fileSize = fileSize;
    node->timeTaken = timeTaken;
    node->next = NULL;
    return node;
}

typedef struct LinkedList {
    Node *head;
    size_t size;
} LinkedList;

//new linked list function
LinkedList *newLinkedList() {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->size = 0;
    return list;
}

// Function to add a new node to the linked list
void addNode(LinkedList *list, size_t fileSize, double timeTaken) {
    Node *node = newNode(fileSize, timeTaken);
    node->next = list->head;
    list->head = node;
    list->size++;
}

// Get the file size index
size_t getFileSizeIndex(LinkedList *list, size_t fileSize) {
    size_t index = 0;
    Node *current = list->head;
    while (current != NULL) {
        if (current->fileSize == fileSize) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

// Get the time taken index
size_t getTimeTakenIndex(LinkedList *list, double timeTaken) {
    size_t index = 0;
    Node *current = list->head;
    while (current != NULL) {
        if (current->timeTaken == timeTaken) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

// Set the file size at the given index
void setFileSize(LinkedList *list, size_t index, size_t fileSize) {
    Node *current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    current->fileSize = fileSize;
}

// Set the time taken at the given index
void setTimeTaken(LinkedList *list, size_t index, double timeTaken) {
    Node *current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    current->timeTaken = timeTaken;
}

// Function to calculate the average time taken to receive the file
double averageTime(LinkedList *list) {
    double sum = 0;
    Node *current = list->head;
    while (current != NULL) {
        sum += current->timeTaken;
        current = current->next;
    }
    return sum / list->size;
}

// Function to calculate the average bandwidth
double averageBandwidth(LinkedList *list) {
    double sum = 0;
    Node *current = list->head;
    while (current != NULL) {
        sum += (current->fileSize / current->timeTaken) * 1000 / (1024 * 1024);
        current = current->next;
    }
    return sum / list->size;
}

// Function to free the linked list
void freeList(LinkedList *list) {
    Node *current = list->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}


/*

*/
void printStatistics(LinkedList *list) {
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");

    Node *current = list->head;
    size_t runNumber = 1;

    while (current != NULL) {
        printf("- Run #%zu Data: Time=%.2fms; Speed=%.2fMB/s\n",
               runNumber, current->timeTaken, (current->fileSize / current->timeTaken) * 1000 / (1024 * 1024));
        current = current->next;
        runNumber++;
    }

    // Calculate and print averages
    double avgTime = averageTime(list);
    double avgBandwidth = averageBandwidth(list);

    printf("- Average time: %.2fms\n", avgTime);
    printf("- Average bandwidth: %.2fMB/s\n", avgBandwidth);
    printf("----------------------------------\n");
}



int main(int argc, char *argv[]) {
    printf("TCP Receiver...\n");


    // Variables
    int sockfd = -1;                            // Socket file descriptor
    struct sockaddr_in server, client;          // Server address structure to store the client and server addresses
    socklen_t clientAddrLen = sizeof(client);   // Client address length
    struct LinkedList *list = newLinkedList();  // Linked list to store the time taken to receive the file and the file size each time

    // Reset the server and client address structures to 0
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    printf("Creating TCP socket...\n");
    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        return 1;
    }

    // The variable to store the socket option for reusing the server's address.
    int opt = 1;

    printf("Setting socket option to reuse server's address...\n");

    // Set the socket option to reuse the server's address.
    // This is useful to avoid the "Address already in use" error message when restarting the server.
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed ");
        close(sockfd);
        return 1;
    }

    printf("Setting server details...\n");

    // Set the servers details
    server.sin_family = AF_INET; // Set the address family to AF_INET (IPv4)
    server.sin_addr.s_addr = INADDR_ANY; // Set the server's IP address to the local IP address
    server.sin_port = htons(atoi(argv[2])); // Set the server's port number to the port number passed as an argument

    printf("Binding socket to server's address...\n");

    // Bind the socket to the server's address
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error on binding");
        close(sockfd);
        return 1;
    }

    printf("Listening for incoming connections...\n");

    // Listen for incoming connections
    if (listen(sockfd, MAXCLIENTS) < 0) {
        perror("Error on listening");
        close(sockfd);
        return 1;
    }

    printf("Server is listening to incoming connections on port %d...\n", atoi(argv[2]));
    printf("Accepting incoming connection...\n");

    size_t fileSize;

    // Accept the incoming connection
    int clientSock = accept(sockfd, (struct sockaddr *)&client, &clientAddrLen);

    if (clientSock < 0) {
        perror("Error on accept");
        close(sockfd);
        return 1;
    }

    printf("Client connected from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    // Step 3: Receive the file size
    size_t bytesRead;
    bytesRead = recv(clientSock, &fileSize, sizeof(fileSize), 0);
    printf("Bytes received: %zu\n", bytesRead);

    if (bytesRead != sizeof(fileSize)) {
        perror("Error receiving file size");
        close(clientSock);
        close(sockfd);
        return 1;
    }


    while (1) {
    printf("File size received: %zu bytes.\n", fileSize);    
    
    // Step 3 (Continued): Receive the file and measure the time
    clock_t start_time, end_time;
    double time_taken;

   char buffer[BUFFER_SIZE];
    size_t totalBytesReceived = 0;

    // Receive the file and measure the time
    printf("Receiving file...\n");
    printf("Starting timer...\n");

    start_time = clock();

    while ((totalBytesReceived < fileSize)){
        //recive data
        bytesRead = recv(clientSock, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            perror("Error receiving file data dfdf");
            close(clientSock);
            close(sockfd);
            return 1;
        }

        if (buffer[BUFFER_SIZE - 1] != '\0')
            buffer[BUFFER_SIZE - 1] = '\0';

        totalBytesReceived += bytesRead;
    }
        printf("Total bytes received: %zu\n", totalBytesReceived);

    //file received so we stop the timer
    printf("File received. Stopping timer...\n");
    end_time = clock();

    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // Convert to milliseconds
    addNode(list, fileSize, time_taken);

    printf("File received in %.2f milliseconds.\n", time_taken);


    // Step 4: Wait for Sender response
    char response[BUFFER_SIZE];
    printf("Waiting for Sender response...\n");

    bytesRead = recv(clientSock, response, sizeof(response), 0);

        if (bytesRead > 0) {
             if (strcmp(response, "exit") == 0) {
                // Sender sends exit message, go to Step 5
                printf("Sender sent exit message. Exiting...\n");
                break;
            }
        } else if (bytesRead == 0) {
            // Connection closed by Sender
            printf("Connection closed by Sender. Exiting...\n");
            break;
        } else {
            //it sent file size agin so we need to set it in filesize
            fileSize = strtoul(response, NULL, 10);
            printf("File size received: %zu bytes.\n", fileSize);
        }
    }

    // Step 5: Print out the times and average bandwidth
    printStatistics(list);

    // Step 6: Close the connections
    close(sockfd);
    freeList(list);
    close(clientSock);
    printf("Server closed.\n");
    printf("Connections closed.\n");



    return 0;
}

