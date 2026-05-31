#ifndef NETWORK_H
#define NETWORK_H

#define BUFFER (256 * 1024) // 256 KiB
#define PORT 1337

typedef unsigned char BYTE;

typedef struct {
    int ID;
    int socket;
} datosCliente;

#endif // NETWORK_H
