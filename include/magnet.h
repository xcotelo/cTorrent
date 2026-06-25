#ifndef MAGNET_H
#define MAGNET_H

#include <stdint.h>
#include <stddef.h>

int parse_magnet(const char *magnet, uint8_t info_hash[20], char *tracker_url);

#endif