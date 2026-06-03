#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "../include/network.h"
#include "../include/protocol.h"

void sendACK(int sock){
    message* ack = create_message(MSG_TYPE_ACK, (const BYTE*)"OK", 2);
    send_message(sock, ack);
    free_message(ack);
}

void* procesarCliente(void *clienteSenProcesar){
    datosCliente* cliente = (datosCliente*) clienteSenProcesar;
    // Paso os datos a local para liberar a estrutura
    int sock = cliente->socket, id = cliente->ID;
    free(cliente);

    while(1){
        message* mensaxeCliente = receive_message(sock);
        if (!mensaxeCliente){
            printf("[Cliente %d] Desconectado\n", id);
            break;
        }

        printf("\n[Cliente %d] Mensaje recibido:\n", id);
        print_message(mensaxeCliente);

        switch(mensaxeCliente->type){
            case MSG_TYPE_TEXT:
                printf("  Texto: %s\n", (char*)mensaxeCliente->payload);
                sendACK(sock);
                break;
                
            case MSG_TYPE_MG_LK:
                printf("  MAGNET LINK: %s\n", (char*)mensaxeCliente->payload);
                sendACK(sock);
                break;

            default:
                printf("  Tipo desconocido\n");
        }
        free_message(mensaxeCliente);
    }
    
    close(sock);
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    int sockfd, socketCliente, contadorClientes = 0;
    pthread_t fio;

    // AF_INET indica IPv4, SOCK_STREAM indica TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    printf("Server is listening...\n");

    while(1){
        // Non me interesa saber a IP nin o porto do cliente
        socketCliente = accept(sockfd, NULL, NULL);

        contadorClientes++;
        
        printf("\nCliente número %d conectado!\n", contadorClientes);
        
        // Preparo datos do cliente
        datosCliente* novoCliente = malloc(sizeof(datosCliente));
        novoCliente->ID = contadorClientes;
        novoCliente->socket = socketCliente;

        if (pthread_create(&fio, NULL, procesarCliente, novoCliente) != 0) {
            perror("Error creating thread");
            close(socketCliente);
            free(novoCliente);
            continue;
        }
        // Sen esperar ao fio, que se vacie só
        pthread_detach(fio);
    }

    close(sockfd);
    return 0;
}