#include "../include/tracker.h"
#include "../include/generate_peer_id.h"
#include "../include/tracker.h"
#include "../include/peer.h"
#include "../include/magnet.h"

int main(int argc, char *argv[])
{
    uint8_t info_hash[20];
    uint8_t peer_id[20];
    char tracker_url[512];

    TrackerPeer *peers = NULL;
    int peer_count = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <magnet-link>\n", argv[0]);
        return 1;
    }

    generate_peer_id(peer_id);
    if (parse_magnet( argv[1], info_hash, tracker_url, sizeof(tracker_url)) < 0)
    {
        fprintf(stderr, "Magnet link inválido\n");
        return 1;
    }

    if (get_peers_from_tracker(tracker_url, info_hash, peer_id, &peers, &peer_count) < 0)
    {
        fprintf(stderr, "Tracker error\n");
        return 1;
    }

    if(peer_count == 0){
        printf("Non se atoparon peers\n");
        return 1;
    }

    int sockfd = connect_to_peer(&peers[0], info_hash, peer_id);

    if (sockfd < 0) {
        fprintf(stderr, "Error ao conectarse ao peer.\n");
    }

    free(peers);

    return 0;
}