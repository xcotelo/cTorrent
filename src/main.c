#include "../include/tracker.h"
#include "../include/generate_peer_id.h"
#include "../include/magnet.h"
#include "../include/banner.h"
#include "../include/worker.h"

int main(int argc, char *argv[])
{
    print_banner();
    uint8_t info_hash[20];
    uint8_t peer_id[20];
    char tracker_url[512];

    TrackerPeer *peers = NULL;
    int peer_count = 0;

    pthread_t threads[NUM_FIOS];
    WorkerData data;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <magnet-link>\n", argv[0]);
        return 1;
    }

    generate_peer_id(peer_id);    
    if (parse_magnet( argv[1], info_hash, tracker_url) < 0)
    {
        fprintf(stderr, "Invalid magnet link\n");
        return 1;
    }

    if (get_peers_from_tracker(tracker_url, info_hash, peer_id, &peers, &peer_count) < 0)
    {
        fprintf(stderr, "Tracker error\n");
        return 1;
    }

    data.peers = peers;
    data.peer_count = peer_count;
    data.info_hash = info_hash;
    data.peer_id = peer_id;

    data.next_peer = 0;

    pthread_mutex_init(&data.mutex, NULL);

    if(peer_count == 0){
        printf("Non se atoparon peers\n");
        return 1;
    }

    printf("\n");
    printf("=============================================================\n");
    printf(" Discovered peers: %d\n", peer_count);
    printf("=============================================================\n\n");


    for (int i = 0; i < peer_count; i++) {
        printf(CYAN "[%03d] " RESET, i);
        print_peer(&peers[i]);
        printf("\n");
    }

    printf("[*] Trying peers...\n");

    for (int i = 0; i < NUM_FIOS; i++) {
        pthread_create(&threads[i], NULL, worker, &data);
    }

    for (int i = 0; i < NUM_FIOS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&data.mutex);
    free(peers);

    return 0;
}