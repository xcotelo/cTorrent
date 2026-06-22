#include "../../include/magnet.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_magnet(const char *magnet, uint8_t info_hash[20], char *tracker_url, size_t tracker_url_size) {
    if (!magnet || !info_hash)
        return -1;

    const char *xt = strstr(magnet, "xt=urn:btih:");

    if (!xt)
        return -1;

    xt += strlen("xt=urn:btih:");

    char hash_str[41];

    strncpy(hash_str, xt, 40);
    hash_str[40] = '\0';

    for (int i = 0; i < 20; i++) {
        unsigned int byte;

        if (sscanf(hash_str + (i * 2), "%2x", &byte) != 1)
            return -1;

        info_hash[i] = (uint8_t)byte;
    }

    if (tracker_url) {
        const char *tr = strstr(magnet, "&tr=");
        if (tr) {
            tr += 4;
            size_t len = strcspn(tr, "&");

            if (len >= tracker_url_size)
                len = tracker_url_size - 1;

            memcpy(tracker_url, tr, len);
            tracker_url[len] = '\0';
        }
        else {
            tracker_url[0] = '\0';
        }
    }

    return 0;
}