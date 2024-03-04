#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345

//
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[1024];
    FILE *file;
    clock_t start, end;
    double cpu_time_used;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    // Set server details
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error on binding");

    // Listen for incoming connections
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    // Accept connection
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("Error on accept");

    // Receive file size first
    long fileSize;
    read(newsockfd, &fileSize, sizeof(fileSize));

    // Receive file content
    file = fopen("received_file.txt", "w");
    start = clock();
    while ((n = read(newsockfd, buffer, sizeof(buffer))) > 0)
        fwrite(buffer, 1, n, file);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Received file in %f seconds\n", cpu_time_used);

    fclose(file);
    close(newsockfd);
    close(sockfd);

    return 0;
}
