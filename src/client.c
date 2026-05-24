#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 1337

int main(){
    struct sockaddr_in server_addr;
    char mensaxe[500];
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_zero[8] = '\0';

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Error connecting to server.");
        return -1;
    }

    while(1){
        printf("Enter you message: ");
        fgets(mensaxe, sizeof(mensaxe), stdin);
        int w = write(sockfd, mensaxe, strlen(mensaxe));
    }

    close(sockfd);
    return 0;
}