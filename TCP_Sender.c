#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define FILE_SIZE_MB 2

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[1024];
    FILE *file;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    // Set server details
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        error("Invalid address");

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error connecting");

    // Generate and send file
    file = fopen("random_file.txt", "w");
    // Generate your random file content here

    // Send file size first
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    write(sockfd, &fileSize, sizeof(fileSize));

    // Send file content
    while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0)
        write(sockfd, buffer, n);

    fclose(file);
    close(sockfd);

    return 0;
}
