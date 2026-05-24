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

#define BUFFER 1024
#define PORT 1337

typedef struct {
    int ID;
    int socket;
} datosCliente;

void* procesarCliente(void *clienteSenProcesar){
    datosCliente* cliente = (datosCliente*) clienteSenProcesar;
    // Paso os datos a local para liberar a estrutura
    int sock = cliente->socket;
    int id = cliente->ID;
    char buffer [BUFFER];
    int bytesRecibidos;

    free(cliente);

    while(1){
        // Inicializo o buffer
        memset(buffer, 0, BUFFER);
        // Gardo os datos recibidos
        bytesRecibidos = recv(sock, buffer, BUFFER, 0);

        if (bytesRecibidos <= 0) {
            printf("\n- [Cliente %d] Desconectado\n", id);
            break;
        }

        printf("+ [Cliente %d] Di: %s", id, buffer);
        
        // Envio o mensaxe de volta
        send(sock, buffer, strlen(buffer), 0);
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

        fio = pthread_create(&fio, NULL, procesarCliente, novoCliente);
        // Sen esperar ao fio, que se vacie só
        pthread_detach(fio);
    }

    close(sockfd);
    return 0;
}