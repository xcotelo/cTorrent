#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/network.h"
#include "../include/protocol.h"

int main(){
    struct sockaddr_in server_addr;
    char mensaxePuro[BUFFER];
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
        fgets(mensaxePuro, sizeof(mensaxePuro), stdin);
        size_t len = strlen(mensaxePuro);

        // Eliminar salto de liña final
        if (len > 0 && mensaxePuro[len-1] == '\n') {
            mensaxePuro[len-1] = '\0';
            len--;
        }

        // Enviar payload en bytes
        message* mensaxe = create_message(MSG_TYPE_TEXT, (const BYTE*)mensaxePuro, (uint32_t)len);
        if (!mensaxe) {
            fprintf(stderr, "Error creating message\n");
            break;
        }
        if (send_message(sockfd, mensaxe) != 0) {
            fprintf(stderr, "Error sending message\n");
            free_message(mensaxe);
            break;
        }
        free_message(mensaxe);

        // Confirmacion (ACK)
        message* resposta = receive_message(sockfd);
        if (!resposta) {
            fprintf(stderr, "Server closed connection or error\n");
            break;
        }
        if (resposta->type == MSG_TYPE_ACK) {
            printf("ACK recibido: %s\n", (char*)resposta->payload);
        } else {
            printf("Resposta recibida (tipo 0x%02X):\n", resposta->type);
            print_message(resposta);
        }
        free_message(resposta);
    }

    close(sockfd);
    return 0;
}