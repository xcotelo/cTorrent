#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include "tracker.h"

#define NUM_FIOS 16

typedef struct {

    TrackerPeer *peers;
    int peer_count;
    const uint8_t *info_hash;
    const uint8_t *peer_id;
    int next_peer;

    pthread_mutex_t mutex;
    pthread_mutex_t print_mutex;
} WorkerData;

void *worker(void *arg);