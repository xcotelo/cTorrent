/*
┌──────┬──────────┬────────────┐
│ TYPE │ LENGTH   │  PAYLOAD   │
│ 1    │ 4 bytes  │ N bytes    │
│ byte │ (uint32) │            │
└──────┴──────────┴────────────┘
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>  // para htonl, ntohl
#include "network.h"

// TYPE
#define MSG_TYPE_TEXT    0x01
#define MSG_TYPE_CMD     0x02
#define MSG_TYPE_FILE    0x03
#define MSG_TYPE_ACK     0x04
#define MSG_TYPE_ERROR   0xFF

// 1 (TYPE) + 4 (LENGTH)
#define HEADER_SIZE 5 

#define MAX_MESSAGE (4 * 1024 * 1024) // 4 MiB

typedef struct {
    uint8_t type;
    uint32_t length;
    BYTE* payload;
} message;

message* create_message(uint8_t type, const BYTE* payload, uint32_t length);
void free_message(message* msg);
int send_message(int sockfd, message* msg);
message* receive_message(int sockfd);
void print_message(message* msg);

#endif // PROTOCOL_H