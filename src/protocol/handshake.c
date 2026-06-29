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
    const uint8_t *ptr = (const uint8_t *)hs;
    size_t total = 0;

    // Había un erro. Os datos poden ter menos bytes.
    // Hai que xestionar isto nun bucle.
    while (total < HANDSHAKE_TOTAL_SIZE) {
        ssize_t sent = send(sockfd, ptr + total, HANDSHAKE_TOTAL_SIZE - total, 0);

        if (sent <= 0) {
            return -1; 
        }

        total += sent;
    }
    return 0;
}


int recv_handshake(int sockfd, Handshake *hs){
    uint8_t *ptr = (uint8_t *)hs;
    size_t total = 0;

    // Había un erro. Os datos non teñen porque vir á vez, poden vir con retraso.
    // Hai que xestionar isto nun bucle.
    while (total < HANDSHAKE_TOTAL_SIZE) {
        ssize_t received = recv(sockfd, ptr + total, HANDSHAKE_TOTAL_SIZE - total, 0);

        if (received <= 0) {
            return -1; 
        }
        total += received;
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