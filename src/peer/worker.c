#include "../../include/worker.h"
#include "../../include/peer.h"
#include "../../include/banner.h"

void *worker(void *arg)
{
    WorkerData *data = arg;

    while (1) {

        pthread_mutex_lock(&data->mutex);

        // Operación CRÍTICA compartida por todos os fios. Hai que bloqueala.
        int i = data->next_peer++;

        pthread_mutex_unlock(&data->mutex);

        if (i >= data->peer_count)
            break;

        TrackerPeer *peer = &data->peers[i];
        char ip[INET_ADDRSTRLEN];
        struct in_addr addr;
        addr.s_addr = peer->ip;
        inet_ntop(AF_INET, &addr, ip, sizeof(ip));

        int sock = connect_to_peer(&data->peers[i], data->info_hash, data->peer_id);

        pthread_mutex_lock(&data->print_mutex);
        printf("[*] [%03d] %s:%u ........ %s\n", i, ip, peer->port, sock >= 0 ? GREEN "OK" RESET : RED "FAIL" RESET);
        pthread_mutex_unlock(&data->print_mutex);

    }

    return NULL;
}