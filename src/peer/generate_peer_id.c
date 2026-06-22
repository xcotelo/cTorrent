#include "../../include/generate_peer_id.h"

static int seeded = 0;

void generate_peer_id(uint8_t *peer_id) {
    if (!seeded) {
        srand(time(NULL) ^ (unsigned long)peer_id);
        seeded = 1;
    }
    const char *prefix = "-TR0001-";
    memcpy(peer_id, prefix, 8);
    
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand(time(NULL) ^ (unsigned long)peer_id);
    
    for (int i = 8; i < PEER_ID_SIZE; i++) {
        peer_id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
}

void print_peer_id(const uint8_t *peer_id) {
    for (int i = 0; i < PEER_ID_SIZE; i++) {
        printf("%02x", peer_id[i]);
    }
}