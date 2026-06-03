#include "protocol.h"
#include "network.h"

message* create_message(uint8_t type, const BYTE *payload, uint32_t length){
    message* mensaxe = (message*)malloc(sizeof(message));
    
    mensaxe->type = type;
    mensaxe->length = length;

    if (length > 0 && payload != NULL){
        mensaxe->payload = (BYTE*)malloc(length + 1);
        if (!mensaxe->payload) {
            free(mensaxe);
            return NULL;
        }
        memcpy(mensaxe->payload, payload, length);
        mensaxe->payload[length] = '\0';
    }else{
        mensaxe->payload = NULL;
    }
    return mensaxe;
}

void free_message(message* msg){
    if (msg && msg->payload){
        free(msg->payload);
        free(msg);
    }
}

int send_message(int sockfd, message* msg){
    uint8_t header[HEADER_SIZE];

    // 1 (TYPE) + 4 (LENGTH)
    header[0] = msg->type;
    
    // 32 bits = 4 bytes
    uint32_t lonxitudeConexion = htonl((uint32_t)msg->length);
    memcpy(&header[1], &lonxitudeConexion, 4);

    send(sockfd, header, HEADER_SIZE, 0);
    // Comprobado con anterioridade se o mensaxe era valido
    send(sockfd, msg->payload, msg->length, 0);

    return 0;
}

message* receive_message(int sockfd) {
    uint8_t header[HEADER_SIZE];
    
    // Recibir header
    int received = recv(sockfd, header, HEADER_SIZE, MSG_WAITALL);
    if (received != HEADER_SIZE) {
        if (received == 0) {
            printf("Conexión cerrada polo cliente\n");
        } else {
            perror("Error receiving header");
        }
        return NULL;
    }
    
    message* msg = (message*)malloc(sizeof(message));
    
    // Extraer a cabeceira
    msg->type = header[0];
    
    // Extraer lonxitude (convertir de network byte order)
    uint32_t net_length;
    memcpy(&net_length, &header[1], 4);
    msg->length = ntohl(net_length);
    
    // Permito ata MAX_MESSAGE(4 MiB) bytes por mensaxe
    if (msg->length > MAX_MESSAGE) {
        fprintf(stderr, "Mensaxe demasiado grande: %u bytes (max %u)\n", msg->length, MAX_MESSAGE);
        free(msg);
        return NULL;
    }    
    if (msg->length > 0) {
        msg->payload = (uint8_t*)malloc((size_t)msg->length + 1);  // +1 para '\0' opcional

        received = recv(sockfd, msg->payload, msg->length, MSG_WAITALL);
        
        // Opcional: añadir terminador para tratar como string
        msg->payload[msg->length] = '\0';

    } else {
        msg->payload = NULL;
    }
    
    return msg;
}

void print_message(message* msg) {
    if (!msg) return;
    
    printf("Tipo: 0x%02X ", msg->type);
    switch(msg->type) {
        case MSG_TYPE_TEXT:   printf("(TEXTO)"); break;
        case MSG_TYPE_MG_LK: printf("(MAGNET LINK)"); break;
        case MSG_TYPE_ACK:    printf("(ACK)"); break;
        default:              printf("(DESCONOCIDO)");
    }
    
    printf("\nLonxitude: %u bytes\n", msg->length);
    
    if (msg->length > 0 && msg->payload) {
        printf("Payload: ");
        if (msg->type == MSG_TYPE_TEXT || msg->type == MSG_TYPE_MG_LK) {
            // Mostrar como texto
            printf("%s\n", (char*)msg->payload);
        } else {
            // Mostrar como hex
            for (uint32_t i = 0; i < msg->length && i < 32; i++) {
                printf("%02X ", msg->payload[i]);
            }
            if (msg->length > 32) printf("...");
            printf("\n");
        }
    }
}