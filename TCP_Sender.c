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
#define FILE_SIZE_MB 3
#define FILE_NAME "random_file.txt"
#define BUFFER_SIZE 1024

/*
    * @brief Create a file with random data.
    * @param fileName The name of the file to create.
    * @param fileSizeInMB The size of the file to create in MB.
    * @return A pointer to the created file.
    
*/
FILE* createRandomCharFile(const char* fileName, size_t fileSizeInMB) {
    // Open the file for writing, create it if it doesn't exist, truncate it if it does
    FILE* file = fopen(fileName, "wb");
    
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Seed for random data
    srand(time(NULL));

    // Write random data to the file
    char buffer[BUFFER_SIZE];
    size_t remainingBytes = fileSizeInMB * 1024 * 1024;

    while (remainingBytes > 0) {
        size_t charsToWrite = sizeof(buffer) < remainingBytes ? sizeof(buffer) : remainingBytes;

          for (size_t i = 0; i < charsToWrite; ++i) {
            buffer[i] = 'A' + rand() % 26;  // Generate a random uppercase letter
        }

        fwrite(buffer, 1, charsToWrite, file);

        remainingBytes -= charsToWrite;
    }

    fclose(file);

    return file;
}



/*
 * @brief TCP Client main function.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments ( 1-2 ip address, 3-4 port, 5-6 congestion control algorithm).
 * @return 0 if the client runs successfully, 1 otherwise.
*/
int main( int argc, char *argv[]) {

    printf("TCP Sender...\n");

    //Variables
    int sockfd = -1;
    char* Server_IP = argv[2];
    char buffer[1024];
    FILE *file;



    printf("Creating file with random data...\n");

    // Create a file with random data
    file = createRandomCharFile(FILE_NAME, FILE_SIZE_MB);







    printf("Creating TCP socket...\n"); 

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("Error opening socket");
        return 1;



    printf("Setting server details...\n");

    // Set server details
    struct sockaddr_in serv_addr;
    // The memset() function is used to fill a block of memory with a particular value. (in this case to reset the serv_addr structure to 0)
    memset(&serv_addr, '0', sizeof(serv_addr));
    // Store the server details in the serv_addr structure.
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(argv[4]);



    printf("Converting IP address from text to binary...\n");

    // Convert the server's address from text to binary form and store it in the server structure.
    // This should not fail if the address is valid.
    if (inet_pton(AF_INET, Server_IP, &serv_addr.sin_addr) <= 0)
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return 1;



    printf("Connecting to server...\n");

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        perror("Connection Failed");
        close(sockfd);
        return 1;




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
    int n;                      // Number of bytes read from the file

    do
    {
        printf("Sending file size...\n");

        // Send file size first so the reciver knows how much data to expect
        // The fseek() function is used to move the file pointer to a specified position. In this case, it moves the file pointer to the end of the file.
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        rewind(file);
        // The write() function is used to write data to a file descriptor. In this case, it writes the file size to the socket.
        write(sockfd, &fileSize, sizeof(fileSize));


        printf("Sending file content...\n");

        // Send file content using chunks of 1024 bytes at a time until the end of the file is reached. 
        // wait for the receiver to acknowledge the receipt of the segment before sending the next segment. 
        while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0)
            write(sockfd, buffer, n);
            

        printf("Do you want to send the file content again? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');


    printf("Sending exit message...\n");

   // Send exit message
    write(sockfd, "exit", 4);


    // Close the file and the socket
    printf("Closing file and socket...\n");
    fclose(file);
    close(sockfd);

    return 0;
}





// TODO: Add the following to the Makefile
// TODO: make sure the argv[] are received correctly (port Ip address and congestion control algorithm)
