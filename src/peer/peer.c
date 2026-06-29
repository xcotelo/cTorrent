#include "../../include/peer.h"
#include "../../include/handshake.h"
#include <sys/time.h> // Para o timeout
#include <errno.h>
#include <fcntl.h>

int connect_to_peer(TrackerPeer *peer, const uint8_t *info_hash, const uint8_t *peer_id){
    int sockfd;
    struct sockaddr_in peer_addr;
    Handshake resposta;
    Handshake envio;
    struct timeval timeout;

    build_handshake(&envio, info_hash, peer_id);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        close(sockfd);
        perror("socket");
        return -1;
    }

    // Inicializo peer_addr
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = peer->ip;
    peer_addr.sin_port = htons(peer->port);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // Aviso ao socket para que deixe de esperar despois de 5 segundos ao recibir e ao enviar (respectivamente)
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    if (connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    if (send_handshake(sockfd, &envio) < 0) {
        close(sockfd);
        return -1;
    }

    if (recv_handshake(sockfd, &resposta) < 0) {
        close(sockfd);
        return -1;
    }

    if (!validate_handshake(&resposta, info_hash)) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}