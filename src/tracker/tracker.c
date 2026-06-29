#include "../../include/tracker.h"

// Convertir números de 64 bits (unsigned long long) a 'network byte order' (O byte máis significativo vai ao principio)
// Serve para enviar
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

// O mesmo ca o de arriba pero replícoo por semántica, para que sexa máis cómodo ler o código
// Serve para recibir
#ifndef ntohll
static inline uint64_t ntohll(uint64_t value) {
    int num = 42;
    if (*(char *)&num == 42) {   // Little endian
        uint32_t high = value >> 32;
        uint32_t low = value & 0xFFFFFFFF;
        return ((uint64_t)ntohl(low) << 32) | ntohl(high);
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
    strncpy(url_copy, tracker_url, sizeof(url_copy) - 1);
    url_copy[sizeof(url_copy) - 1] = '\0';
    
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
    host[sizeof(host) - 1] = '\0';
    port = atoi(host_porto + 1);
    
    // Quitar /announce se o hai
    char *slash = strchr(host, '/');
    if (slash) {
        *slash = '\0';
    }
    
    printf("\n[*] Connecting to tracker...\n");
    printf("[+] Tracker: %s:%d\n", host, port); 

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
// "Antes de anunciar, débese obter un connection_id temporal"
static uint64_t get_connection_id(int sockfd, struct sockaddr_in *tracker_addr) {
    uint8_t connect_req[16];
    uint8_t connect_resp[16];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    //  1. Choose a random transaction ID
    //  2. Fill the connect request structure.
    memset(connect_req, 0, sizeof(connect_req));

    /* Protocol ID */
    uint64_t protocol_id = htonll(0x41727101980ULL); // Numero identificativo para que o tracker saiba que falo UDP Tracker

    /* Action = CONNECT */
    uint32_t action = htonl(0);

    /* Transaction ID */
    uint32_t transaction_id = (uint32_t)rand();
    uint32_t transaction_id_net = htonl(transaction_id);

    memcpy(connect_req + 0,  &protocol_id,        8);
    memcpy(connect_req + 8,  &action,             4);
    memcpy(connect_req + 12, &transaction_id_net, 4);
    // 8 + 4 + 4 = 16 bytes que xustamente é o tamaño do paquete CONNECT

    // 3. Send the packet. (con reitentos)
    ssize_t recv_len = -1;
    int timeout = 15;

    for (int intento = 0; intento < 5; intento++) {

        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        setsockopt(sockfd, SOL_SOCKET,
                SO_RCVTIMEO,
                &tv,
                sizeof(tv));

        if (sendto(sockfd,
                connect_req,
                sizeof(connect_req),
                0,
                (struct sockaddr *)tracker_addr,
                sizeof(*tracker_addr)) < 0) {
            perror("sendto connect");
            return 0;
        }

        recv_len = recvfrom(sockfd,
                            connect_resp,
                            sizeof(connect_resp),
                            0,
                            (struct sockaddr *)&from_addr,
                            &from_len);

        if (recv_len >= 0) {
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("[*] Timeout. Retrying CONNECT (%d)...\n", timeout);
            timeout *= 2;
            continue;
        }

        perror("recvfrom connect");
        return 0;
    }

    if (recv_len < 0) {
        fprintf(stderr, "CONNECT fallou tras 5 intentos\n");
        return 0;
    }
    
    // 4. Check whether the packet is at least 16 bytes
    if (recv_len != 16) {
        fprintf(stderr, "Error: Resposta de conexión de tamaño incorrecto (mínimo 16 bytes): %zd\n", recv_len);
        return 0;
    }

    // 5. Check whether the transaction ID is equal to the one you chose.
    uint32_t response_action;
    uint32_t response_transaction;
    uint64_t connection_id;

    memcpy(&response_action, connect_resp + 0, 4);
    memcpy(&response_transaction, connect_resp + 4, 4);
    memcpy(&connection_id, connect_resp + 8, 8);

    response_action = ntohl(response_action);
    response_transaction = ntohl(response_transaction);
    connection_id = ntohll(connection_id);

    // 6. Check whether the action is connect.
    if (response_action != 0) {
        fprintf(stderr, "Error: Non está conectado. Action non é 0: %u\n", action);
        return 0;
    }
    
    if (response_transaction != transaction_id) {
        fprintf(stderr, "CONNECT transaction ID mismatch.\n");
        return 0;
    }

    // 7. Store the connection ID for future use.
    printf("[+] Connection ID: %llu\n", (unsigned long long)connection_id);    
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
    
    uint32_t announce_transaction = (uint32_t)rand();
    req.connection_id = htonll(connection_id);  // O protocolo UDP Tracker obriga a envialo en 'network byte order'
    req.action = htonl(1);  // 1= ANNOUNCE
    req.transaction_id = htonl(announce_transaction); // Aleatorio
    memcpy(req.info_hash, info_hash, 20);
    memcpy(req.peer_id, peer_id, 20);
    req.downloaded = htonll(0);
    req.left = htonll(1000000000);  // 1GB (por ahora)
    req.uploaded = htonll(0);
    req.event = htonl(2);  // started
    req.ip = 0;
    req.key = htonl(rand());
    req.num_want = htonl(-1);
    req.port = htons(PEER_PORT);
    
    // 5. Enviar ANNOUNCE con reintentos
    uint8_t response[2048];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    int timeout = 15;   // BEP-15: empezar con 15 segundos
    int recv_len = -1;

    for (int intento = 0; intento < 8; intento++) {

        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (sendto(sockfd, &req, sizeof(req), 0,
                (struct sockaddr *)&tracker_addr,
                sizeof(tracker_addr)) < 0) {
            perror("sendto announce");
            close(sockfd);
            return -1;
        }

        recv_len = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr *)&from_addr, &from_len);

        if (recv_len >= 0) {
            break;          // resposta recibida
        }

        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom announce");
            close(sockfd);
            return -1;
        }

        printf("[*] Timeout. Retrying ANNOUNCE (%d)...\n", intento + 1);

        timeout *= 2;       // 15 -> 30 -> 60 -> 120 -> ...
    }

    if (recv_len < 0) {
        fprintf(stderr, "Error Tracker.\n");
        close(sockfd);
        return -1;
    }
    
    // 7. Parsear resposta
    if (parse_tracker_response(response, recv_len, announce_transaction, peers, peer_count) < 0) {
        close(sockfd);
        return -1;
    }
    
    close(sockfd);
    return 0;
}

// Parsear respuesta do tracker
    int parse_tracker_response(const uint8_t *response, size_t len, uint32_t expected_transaction,
                           TrackerPeer **peers, int *peer_count) {
    if (len < 20) {
        fprintf(stderr, "Error: short message\n");
        return -1;
    }
    
    uint32_t action = ntohl(*(uint32_t*)(response + 0));
    uint32_t transaction = ntohl(*(uint32_t *)(response + 4));
    
    if (transaction != expected_transaction) {
        fprintf(stderr, "Transaction ID mismatch.\n");
        return -1;
    }

    if (action == 3) {  // Error
        char error_msg[256];
        strncpy(error_msg, (char*)(response + 8), len - 8);
        error_msg[len - 8] = '\0';
        fprintf(stderr, "Tracker error: %s\n", error_msg);
        return -1;
    }
    
    // Non pode ser ANNOUNCE
    if (action != 1) { 
        fprintf(stderr, "Error: Invalid Action: %u\n", action);
        return -1;
    }
    
    // Extraer valores
    uint32_t interval = ntohl(*(uint32_t*)(response + 8));
    uint32_t leechers = ntohl(*(uint32_t*)(response + 12));
    uint32_t seeders = ntohl(*(uint32_t*)(response + 16));
    
    printf("[+] Announce interval: %u seconds\n", interval);
    printf("[+] Leechers: %u\n", leechers);
    printf("[+] Seeders: %u\n", seeders);
    
    // Os peers empezan en byte 20
    const uint8_t *peer_data = response + 20;
    size_t peer_data_len = len - 20;

    // Os peers ocupan EXACTAMENTE 6 bytes na resposta (4 IP + 2 PORTO)
    if (peer_data_len % 6 != 0) {
        fprintf(stderr, "Peer's data size invalid: %zu\n", peer_data_len);
    }
    
    *peer_count = peer_data_len / 6; // Número de peers que existen
    *peers = malloc((*peer_count) * sizeof(TrackerPeer));
    if (!*peers) {
        perror("malloc");
        return -1;
    }
    
    // Extraer información de cada peer 
    for (int i = 0; i < *peer_count; i++) {
        const uint8_t *p = peer_data + (i * 6); // Inicio peer
        memcpy(&(*peers)[i].ip, p, 4);
        (*peers)[i].port = ntohs(*(uint16_t*)(p + 4));
    }
    
    return 0;
}

// Función auxiliar para imprimir IP
void print_peer(const TrackerPeer *peer) {
    struct in_addr addr;
    addr.s_addr = peer->ip;
    // Para convirtir a IP en notación ipv4
    printf("%s:%d", inet_ntoa(addr), peer->port);
}