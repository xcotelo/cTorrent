#include "../../include/tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <time.h>

// Implementar htonll
#ifndef htonll
static inline uint64_t htonll(uint64_t value) {
    int num = 42;
    if (*(char *)&num == 42) {  // Little endian
        uint32_t high = value >> 32;
        uint32_t low = value & 0xFFFFFFFF;
        return ((uint64_t)htonl(low) << 32) | htonl(high);
    }
    return value;
}
#endif

// Función para conectar a tracker UDP
int connect_to_tracker(const char *tracker_url, struct sockaddr_in *tracker_addr) {
    char host[256];
    int port;
    
    // Parsear URL: tracker.com:6969 o tracker.com:6969/announce
    char url_copy[256];
    strncpy(url_copy, tracker_url, sizeof(url_copy));
    
    // Quitar "udp://" se existe
    char *url = url_copy;
    if (strncmp(url, "udp://", 6) == 0) {
        url += 6;
    }
    
    // Separar host:porto
    char *host_porto = strchr(url, ':');
    if (!host_porto) {
        fprintf(stderr, "Error: URL sen porto: %s\n", url);
        return -1;
    }
    
    *host_porto = '\0';
    strncpy(host, url, sizeof(host) - 1);
    port = atoi(host_porto + 1);
    
    // Quitar /announce se o hai
    char *slash = strchr(host, '/');
    if (slash) {
        *slash = '\0';
    }
    
    printf("Conectando a tracker: %s:%d\n", host, port);
    
    // Resolver DNS
    struct hostent *server = gethostbyname(host);
    if (!server) {
        fprintf(stderr, "Error: Non se pode resolver %s\n", host);
        return -1;
    }
    
    memset(tracker_addr, 0, sizeof(*tracker_addr));
    tracker_addr->sin_family = AF_INET;
    memcpy(&tracker_addr->sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    tracker_addr->sin_port = htons(port);
    
    return 0;
}

// Función para obter connection_id (handshake con tracker).
// Isto está documentado en: https://www.bittorrent.org/beps/bep_0015.html
// "Antes de anunciar, débese obter un connection_id."
static uint64_t get_connection_id(int sockfd, struct sockaddr_in *tracker_addr) {
    uint8_t connect_req[16];
    uint8_t connect_resp[16];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    uint32_t action = htonl(0);

    //  1. Choose a random transaction ID
    //  2. Fill the connect request structure.
    memset(connect_req, 0, sizeof(connect_req));
    uint64_t protocol_id = htonll(0x41727101980ULL);  // Magic constant
    memcpy(connect_req, &protocol_id, 8);
    memcpy(connect_req + 8, &action, 4);       // Action = connect
    memcpy(connect_req + 12, &transaction_id_net, 4); // Transaction ID
    
    // 3. Send the packet.
    if (sendto(sockfd, connect_req, sizeof(connect_req), 0,  (struct sockaddr*)tracker_addr, 
    sizeof(*tracker_addr)) < 0) {
        perror("sendto connect");
        return 0;
    }
    
    printf("Enviada solicitude de conexión ao tracker\n");
    
    // 3. Receive the packet.
    ssize_t recv_len = recvfrom(sockfd, connect_resp, sizeof(connect_resp), 0,
                                (struct sockaddr*)&from_addr, &from_len);
    if (recv_len < 0) {
        perror("recvfrom connect");
        return 0;
    }
    
    // 4. Check whether the packet is at least 16 bytes
    if (recv_len != 16) {
        fprintf(stderr, "Error: Resposta de conexión de tamaño incorrecto (mínimo 16 bytes): %zd\n", recv_len);
        return 0;
    }
    
    // 5. Check whether the transaction ID is equal to the one you chose.
    uint32_t action = ntohl(*(uint32_t*)(connect_resp + 0));
    uint32_t transaction = ntohl(*(uint32_t*)(connect_resp + 4));
    
    // 6. Check whether the action is connect.
    if (action != 0) {
        fprintf(stderr, "Error: Non está conectado. Action non é 0: %u\n", action);
        return 0;
    }
    
    // 7. Store the connection ID for future use.
    uint64_t connection_id = *(uint64_t*)(connect_resp + 8);
    printf("Connection ID obtenido: %llu\n", (unsigned long long)connection_id);
    
    return connection_id;
}

// Función principal
int get_peers_from_tracker(const char *tracker_url, const uint8_t *info_hash, const uint8_t *peer_id,
                           TrackerPeer **peers, int *peer_count) {
    
    struct sockaddr_in tracker_addr;

    if (!tracker_url || !info_hash || !peer_id) {
        fprintf(stderr, "Error: Parámetros inválidos\n");
        return -1;
    }
    
    // 1. Crear socket UDP (SOCK_DGRAM)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    // 2. Conectar ao tracker
    if (connect_to_tracker(tracker_url, &tracker_addr) < 0) {
        close(sockfd);
        return -1;
    }
    
    // 3. Obter connection_id
    uint64_t connection_id = get_connection_id(sockfd, &tracker_addr);
    if (connection_id == 0) {
        close(sockfd);
        return -1;
    }
    
    // 4. Construir announce request
    AnnounceRequest req;
    memset(&req, 0, sizeof(req));
    
    req.connection_id = connection_id;
    req.action = htonl(1);  // 1= ANNOUNCE
    req.transaction_id = htonl(rand()); // Aleatorio
    memcpy(req.info_hash, info_hash, 20);
    memcpy(req.peer_id, peer_id, 20);
    req.downloaded = 0;
    req.left = htonll(1000000000);  // 1GB (por ahora)
    req.uploaded = 0;
    req.event = htonl(2);  // started
    req.ip = 0;
    req.key = htonl(rand());
    req.num_want = htonl(-1);
    req.port = htons(PEER_PORT);
    
    // 5. Enviar announce
    if (sendto(sockfd, &req, sizeof(req), 0, 
               (struct sockaddr*)&tracker_addr, sizeof(tracker_addr)) < 0) {
        perror("sendto announce");
        close(sockfd);
        return -1;
    }
    
    printf("ANNOUNCE enviado ao tracker\n");
    
    // 6. Recibir resposta
    uint8_t response[2048];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    ssize_t recv_len = recvfrom(sockfd, response, sizeof(response), 0, 
    (struct sockaddr*)&from_addr, &from_len);
    if (recv_len < 0) {
        perror("recvfrom announce");
        close(sockfd);
        return -1;
    }
    
    // 7. Parsear resposta
    if (parse_tracker_response(response, recv_len, peers, peer_count) < 0) {
        close(sockfd);
        return -1;
    }
    
    close(sockfd);
    return 0;
}

// Parsear respuesta do tracker
int parse_tracker_response(const uint8_t *response, size_t len, 
                           TrackerPeer **peers, int *peer_count) {
    if (len < 20) {
        fprintf(stderr, "Error: Resposta demasiado curta\n");
        return -1;
    }
    
    uint32_t action = ntohl(*(uint32_t*)(response + 0));
    uint32_t transaction = ntohl(*(uint32_t*)(response + 4));
    
    if (action == 3) {  // Error
        char error_msg[256];
        strncpy(error_msg, (char*)(response + 8), len - 8);
        error_msg[len - 8] = '\0';
        fprintf(stderr, "Error do tracker: %s\n", error_msg);
        return -1;
    }
    
    // Non pode ser ANNOUNCE
    if (action != 1) { 
        fprintf(stderr, "Error: Action incorrecto: %u\n", action);
        return -1;
    }
    
    // Extraer valores
    uint32_t interval = ntohl(*(uint32_t*)(response + 8));
    uint32_t leechers = ntohl(*(uint32_t*)(response + 12));
    uint32_t seeders = ntohl(*(uint32_t*)(response + 16));
    
    printf("Intervalo: %u segundos\n", interval);
    printf("Leechers: %u\n", leechers);
    printf("Seeders: %u\n", seeders);
    
    // Os peers empezan en byte 20
    const uint8_t *peer_data = response + 20;
    size_t peer_data_len = len - 20;

    // Os peers ocupan EXACTAMENTE 6 bytes na resposta (4 IP + 2 PORTO)
    if (peer_data_len % 6 != 0) {
        fprintf(stderr, "Datos de peers de tamaño incorrecto: %zu\n", peer_data_len);
    }
    
    *peer_count = peer_data_len / 6;
    *peers = malloc((*peer_count) * sizeof(TrackerPeer));
    if (!*peers) {
        perror("malloc");
        return -1;
    }
    
    // Extraer información de cada peer 
    for (int i = 0; i < *peer_count; i++) {
        const uint8_t *p = peer_data + (i * 6); // Inicio peer
        (*peers)[i].ip = *(uint32_t*)p;
        (*peers)[i].port = ntohs(*(uint16_t*)(p + 4));
    }
    
    return 0;
}

// Función auxiliar para imprimir IP
void print_peer(const TrackerPeer *peer) {
    struct in_addr addr;
    addr.s_addr = peer->ip;
    printf("%s:%d", inet_ntoa(addr), peer->port);
}