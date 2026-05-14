#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> 
#include <arpa/inet.h>

int main() {
    struct sockaddr_in server_addr;
    struct sockaddr_in con_addr;
    int struct_size;
    int clientfd;
    int bytes_read;
    int sockfd;
    char buf[50];
    socklen_t addr_len;

    // AF_INET indica IPv4, SOCK_STREAM indica TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    while(1){
        addr_len = sizeof(con_addr);
        clientfd = accept(sockfd, (struct sockaddr*)&con_addr, &addr_len);

        while((bytes_read = recv(clientfd, buf, sizeof(buf) - 1, 0)) > 0){
            if (bytes_read > 0){
                buf[bytes_read] = '\0';
                printf("Message from client:%d is %s \n",clientfd, buf);
            }
        }
        close(clientfd);
    }
    close(sockfd);
    return 0;
}