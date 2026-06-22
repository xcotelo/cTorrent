// Vai conter os datos do Peer conectado a diferenza do de "tracker.h"

#ifndef PEER_H
#define PEER_H

#include "tracker.h"
#include <unistd.h>

int connect_to_peer(TrackerPeer *peer, const uint8_t *info_hash, const uint8_t *peer_id);

#endif