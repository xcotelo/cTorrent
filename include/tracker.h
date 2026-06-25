#ifndef TRACKER_H
#define TRACKER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <time.h>

#define TRACKER_ANNOUNCE_SIZE 16  // Tamaño mínimo de ANNOUNCE
#define PEER_PORT 6881

// Estrutura dun peer
typedef struct {
    uint32_t ip;
    uint16_t port;
} TrackerPeer;

// Estructura UDP (según BEP 15: https://www.bittorrent.org/beps/bep_0015.html)
typedef struct {
    uint64_t connection_id;   // 8 bytes
    uint32_t action;          // 0 = connect, 1 = announce
    uint32_t transaction_id;  // 4 bytes
    uint8_t info_hash[20];    // 20 bytes
    uint8_t peer_id[20];      // 20 bytes
    uint64_t downloaded;      // 8 bytes
    uint64_t left;            // 8 bytes (bytes restantes)
    uint64_t uploaded;        // 8 bytes
    uint32_t event;           // 0 = none, 1 = completed, 2 = started, 3 = stopped
    uint32_t ip;              // 0 = default
    uint32_t key;             // 4 bytes (random)
    uint32_t num_want;        // -1 = default
    uint16_t port;            // 2 bytes
} AnnounceRequest;

typedef struct {
    uint32_t action;
    uint32_t transaction_id;
    uint32_t interval;
    uint32_t leechers;
    uint32_t seeders;
} AnnounceResponse;


int get_peers_from_tracker(const char *tracker_url, const uint8_t *info_hash, const uint8_t *peer_id,
                           TrackerPeer **peers, int *peer_count);

int parse_tracker_response(const uint8_t *response, size_t len, uint32_t expected_transaction, TrackerPeer **peers, int *peer_count);

// Función auxiliar para imprimir IP
void print_peer(const TrackerPeer *peer);

#endif