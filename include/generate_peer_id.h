#ifndef PEER_ID_H
#define PEER_ID_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PEER_ID_SIZE 20

void generate_peer_id(uint8_t *peer_id);
void print_peer_id(const uint8_t *peer_id);

#endif