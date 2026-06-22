#include "../../include/handshake.h"
#include <stdio.h>
#include <unistd.h>

void build_handshake(Handshake *hs, const uint8_t *info_hash, const uint8_t *peer_id) {
    hs->pstrlen = HANDSHAKE_PROTOCOLLEN;
    
    memcpy(hs->pstr, HANDSHAKE_PROTOCOL, HANDSHAKE_PROTOCOLLEN);
    
    memset(hs->reserved, 0, HANDSHAKE_RESERVED_SIZE);
    
    memcpy(hs->info_hash, info_hash, HASH_SIZE);
    
    memcpy(hs->peer_id, peer_id, PEER_ID_SIZE);
}

int send_handshake(int sockfd, const Handshake *hs){
    ssize_t sent = send(sockfd, hs, HANDSHAKE_TOTAL_SIZE, 0);

    if (sent != HANDSHAKE_TOTAL_SIZE) {
        close(sockfd);
        return -1; 
    }
    return 0;
}

int recv_handshake(int sockfd, Handshake *hs){
    ssize_t received = recv(sockfd, hs, HANDSHAKE_TOTAL_SIZE, 0);

    if (received != HANDSHAKE_TOTAL_SIZE) {
        close(sockfd);
        return -1; 
    }
    return 0;
}

bool validate_handshake(const Handshake *hs, const uint8_t *expected_hash) {
    if (hs->pstrlen != HANDSHAKE_PROTOCOLLEN) {
        return false;
    }
    
    if (memcmp(hs->pstr, HANDSHAKE_PROTOCOL, HANDSHAKE_PROTOCOLLEN) != 0) {
        return false;
    }
    
    if (memcmp(hs->info_hash, expected_hash, HASH_SIZE) != 0) {
        return false;
    }
    
    return true;
}