#include <sys/socket.h> 
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 

int main(int argc, char **argv) {
    char buf[256];
    socklen_t len;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
 	if (sock == -1) {
     		perror("socket");
        return -1;
    } 
    
    len = sizeof(buf); 
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) { 
        perror("getsockopt");
        return -1;
    } 
    
    printf("Current TCP CONGESTION CONTROL Algorithm: %s\n", buf); 
    
    
    strcpy(buf, "reno"); 
    len = strlen(buf);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, len) != 0) {
        perror("setsockopt"); 
        return -1;
    }
    len = sizeof(buf); 
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) {
        perror("getsockopt"); 
        return -1; 
    } 
    printf("New TCP CONGESTION CONTROL Algorithm: %s\n", buf); 
    close(sock); 
    return 0; 
}
