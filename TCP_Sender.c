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

#define PORT 12345
#define FILE_SIZE_MB 2
#define BUFFER_SIZE 65536

/*
    * @brief Create a file with random data.
    * @param fileName The name of the file to create.
    * @param fileSizeInMB The size of the file to create in MB.
    * @return A pointer to the created file.
    
*/
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



/*
 * @brief TCP Client main function.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments ( 1-2 ip address, 3-4 port, 5-6 congestion control algorithm).
 * @return 0 if the client runs successfully, 1 otherwise.
*/
int main(int argc, char *argv[]) {

    printf("TCP Sender...\n");

    //Variables
    int sockfd = -1;
    char* Server_IP = argv[2];
    char *Message;

    printf("Creating file with random data...\n");

    // Create a file with random data
    Message = util_generate_random_data(FILE_SIZE_MB *1024 * 1024);







    printf("Creating TCP socket...\n"); 

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Error opening socket");
        return 1;
    }


    printf("Setting server details...\n");

    // Set server details
    struct sockaddr_in serv_addr;
    // The memset() function is used to fill a block of memory with a particular value. (in this case to reset the serv_addr structure to 0)
    memset(&serv_addr, '0', sizeof(serv_addr));

    // Validate and convert port from command line
    int port = atoi(argv[4]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        close(sockfd);
        return 1;
    }

    // Store the server details in the serv_addr structure.
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);



    printf("Converting IP address from text to binary...\n");

    // Convert the server's address from text to binary form and store it in the server structure.
    // This should not fail if the address is valid.
    if (inet_pton(AF_INET, Server_IP, &serv_addr.sin_addr) <= 0){
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return 1;
    }



    printf("Connecting to server...\n");

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Connection Failed");
        close(sockfd);
        return 1;
    }




    printf("Setting congestion control algorithm...\n");

     // Define congestion control algorithm
    if (strcmp(argv[6], "reno") == 0) {
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) < 0)
            perror("Error setting congestion control algorithm");
    } else if (strcmp(argv[6], "cubic") == 0) {
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic")) < 0)
            perror("Error setting congestion control algorithm");
    } else {
        perror("Invalid congestion control algorithm");
        close(sockfd);
        return 1;
    }
    

    // Loop to send the file content agin if the user wants to send the file content again (enter y = yes, n = no)
    char choice = 'y';          // User's choice
    //int n;                      // Number of bytes read from the file

        size_t fileSize = FILE_SIZE_MB * 1024 * 1024;
    do
    {
        printf("Sending file size...\n");

        
        // The write() function is used to write data to a file descriptor. In this case, it writes the file size to the socket.
        write(sockfd, &fileSize, sizeof(fileSize));
        //claen the buffer of the socket


        printf("Sending file content...\n");


    // Loop to send the string in chunks
    for (size_t i = 0; i < fileSize; i += BUFFER_SIZE) {
        // Calculate the remaining characters to send in this iteration
        size_t remaining = fileSize - i;
        // Determine the size to send in this iteration (up to BUFFER_SIZE)
        size_t chunkSize = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

        // Write the current chunk to the socket
        write(sockfd, Message + i, chunkSize);
    }

        printf("Do you want to send the file content again? (y/n): ");
        scanf(" %c", &choice);
        while (getchar() != '\n'); // Clear the input buffer
    } while (choice == 'y' || choice == 'Y');


    printf("Sending exit message...\n");

   // Send exit message
    write(sockfd, "exit", 4);


    // Close the file and the socket
    printf("Closing file and socket...\n");
    // fclose(file);
    close(sockfd);

    return 0;
}
