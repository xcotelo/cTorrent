#include "../include/tracker.h"
#include "../include/generate_peer_id.h"
#include "../include/peer.h"
#include "../include/magnet.h"
#include "../include/banner.h"

int main(int argc, char *argv[])
{
    print_banner();
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
    if (parse_magnet( argv[1], info_hash, tracker_url) < 0)
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

    printf("\n");
    printf("=============================================================\n");
    printf(" Peers atopados : %d\n", peer_count);
    printf("=============================================================\n\n");


    for (int i = 0; i < peer_count; i++) {
        printf(CYAN "[%03d] " RESET, i);
        print_peer(&peers[i]);
        printf("\n");
    }
    
    int sockfd = connect_to_peer(&peers[0], info_hash, peer_id);
    
    close(sockfd);
    free(peers);

    return 0;
}