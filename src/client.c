#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/datos.h"

int main(){
    struct sockaddr_in server_addr;
    char mensaxe[500];
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Error connecting to server.");
        return -1;
    }

    while(1){
        printf("Enter you message: ");
        fgets(mensaxe, sizeof(mensaxe), stdin);

        size_t len = strlen(mensaxe);
        // Eliminar salto de liña final
        if (len > 0 && mensaxe[len-1] == '\n') {
            len--;
        }

        if (len == 0) continue;

        BYTE mensaxeByte[len];
        for (size_t i = 0; i < len; ++i){
            mensaxeByte[i] = (BYTE)mensaxe[i];
        }

        ssize_t sent = write(sockfd, mensaxeByte, len);
    }

    close(sockfd);
    return 0;
}