#include "../../include/peer.h"
#include "../../include/handshake.h"

int connect_to_peer(TrackerPeer *peer, const uint8_t *info_hash, const uint8_t *peer_id){
    int sockfd;
    struct sockaddr_in peer_addr;
    Handshake resposta;
    Handshake envio;

    build_handshake(&envio, info_hash, peer_id);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // Inicializo peer_addr
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = peer->ip;
    peer_addr.sin_port = htons(peer->port);

    if (connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }    

    send_handshake(sockfd, &envio);
    recv_handshake(sockfd, &resposta);
    validate_handshake(&resposta, info_hash);

    return sockfd;
}