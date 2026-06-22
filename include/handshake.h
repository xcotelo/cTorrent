/*
┌───────────┬───────────┬───────────────────────────────────────────────┐
| Campo	    | Tamaño	| Descripción                                   | 
| pstrlen	| 1 byte	| Longitud del protocolo (siempre 19)           | 
| pstr	    | 19 bytes	| "BitTorrent protocol"                         | 
| reserved  | 8 bytes	| Reservado (ceros, para extensiones futuras)   | 
| info_hash | 20 bytes	| SHA-1 del torrent (de la semana 4)            | 
| peer_id   | 20 bytes	| Identificador único do meu cliente            | 
└───────────┴───────────┴───────────────────────────────────────────────┘

<pstrlen=19><pstr="BitTorrent protocol"><reserved=8x0><info_hash=20b><peer_id=20b>

*/

#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>  // para htonl, ntohl

#define HANDSHAKE_PROTOCOLLEN 19
#define HANDSHAKE_PROTOCOL "BitTorrent protocol"
#define HANDSHAKE_RESERVED_SIZE 8
#define HASH_SIZE 20
#define PEER_ID_SIZE 20
#define HANDSHAKE_TOTAL_SIZE 68  // 1 + 19 + 8 + 20 + 20

typedef struct {
    uint8_t pstrlen;
    char pstr[19];
    uint8_t reserved[8];
    uint8_t info_hash[20];
    uint8_t peer_id[20];
} Handshake;

void build_handshake(Handshake *hs, const uint8_t *info_hash, const uint8_t *peer_id);

int send_handshake(int sockfd, const Handshake *hs);

int recv_handshake(int sockfd, Handshake *hs);

bool validate_handshake(const Handshake *hs, const uint8_t *expected_hash);

#endif