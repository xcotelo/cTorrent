# cTorrent

<p align="center">
  <img src="img/logo.png" alt="Logo de cTorrent">
</p>

![Language](https://img.shields.io/badge/language-C-blue)
![Build](https://img.shields.io/badge/build-Make-success)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)

cTorrent es un cliente BitTorrent desarrollado desde cero en C con fines educativos y de investigación en sistemas distribuidos y programación de redes.

El objetivo principal del proyecto es comprender e implementar manualmente los componentes fundamentales del protocolo BitTorrent sin depender de bibliotecas especializadas ni clientes existentes.

Características implementadas

* Parsing de Magnet Links
* Comunicación con trackers UDP (BEP 15)
* Descubrimiento de peers
* Generación de Peer IDs
* Handshake BitTorrent
* Programación de sockets POSIX

Las siguientes fases incluirán:

* Intercambio de mensajes BitTorrent
* Descarga de piezas
* Verificación SHA-1
* Gestión simultánea de múltiples peers
* DHT (Distributed Hash Table)
* Algoritmos de selección de piezas
* Scheduler de descargas

---

## Motivación

BitTorrent es uno de los protocolos P2P más relevantes de la historia de Internet.

Aunque existen numerosos clientes maduros, pocos desarrolladores han implementado personalmente:

* Descubrimiento de peers
* Protocolos binarios
* Comunicación tracker-peer
* Gestión de sockets a bajo nivel
* Handshakes de protocolos reales

cTorrent nace como un proyecto de aprendizaje para profundizar en:

* Sistemas operativos
* Redes TCP/IP
* Protocolos distribuidos
* Concurrencia
* Diseño de software en C

---

## Tecnologías utilizadas

Lenguaje:

* C

Herramientas:

* GCC
* Make
* Git
* GDB

APIs del sistema:

* POSIX Sockets
* TCP
* UDP

Conceptos aplicados:

* Network Programming
* Binary Protocols
* Endianness
* DNS Resolution
* Memory Management
* Peer-to-Peer Architectures

---

## Arquitectura

```text
                  Magnet Link
                       │
                       ▼
              Parse Metadata
                       │
                       ▼
              Resolve Tracker DNS
                       │
                       ▼
             UDP Tracker (BEP 15)
                       │
                       ▼
                Peer Discovery
                       │
                       ▼
             TCP Peer Connection
                       │
                       ▼
            BitTorrent Handshake
                       │
                       ▼
         Peer Wire Protocol Messages
                       │
                       ▼
              Piece Download Engine
```

---

## Estructura del proyecto

```text
cTorrent/
│
├── img/
│   ├── logo.png
│
├── include/
│   ├── generate_peer_id.h
│   ├── handshake.h
│   ├── magnet.h
│   ├── network.h
│   ├── peer.h
│   ├── protocol.h
│   └── tracker.h
│   └── banner.h
│   └── url_decode.h
│
├── src/
│   ├── main.c
│   │
│   ├── peer/
│   │   ├── generate_peer_id.c
│   │   └── peer.c
│   │
│   ├── protocol/
│   │   ├── handshake.c
│   │   └── protocol.c
│   │
│   ├── torrent/
│   │   └── magnet.c
│   │
│   └── tracker/
│       └── tracker.c
│       └── url_decode.c
│
├── Makefile
└── README.md
```

---

## Componentes implementados

### UDP Tracker Client (BEP 15)

Implementación manual del protocolo UDP Tracker.

Flujo:

1. Resolución DNS
2. Connect Request
3. Obtención de Connection ID
4. Announce Request
5. Recepción de peers

Características

* DNS Resolution
* UDP Connect Request
* Connection ID Negotiation
* Announce Request
* Peer Parsing
* Compact Peer List Parsing

---

### Peer Discovery

Una vez recibido el announce response:

```text
Tracker
    │
    ▼
Peer List
```

Cada peer se almacena mediante:

```c
typedef struct {
    uint32_t ip;
    uint16_t port;
} TrackerPeer;
```

---

### BitTorrent Handshake

Implementación del handshake oficial de BitTorrent:

```text
<pstrlen>
<pstr>
<reserved>
<info_hash>
<peer_id>
```

Tamaño total:

```text
68 bytes
```

Permite verificar:

* Compatibilidad del protocolo
* Coincidencia del info_hash
* Identificación de peers

---

### Peer ID Generation

Generación automática de identificadores únicos para cada instancia del cliente.

Formato:

```text
20 bytes
```

Utilizado durante:

* Tracker Announce
* Handshake BitTorrent

---

## Compilación

```bash
make
```

Genera:

```text
compilation/ctorrent
```

---

## Ejecución

Ejemplo:

```bash
./compilation/ctorrent \
"magnet:?xt=urn:btih:FE60B29767946ECDCD087E5AC9E66E480C4755D1&dn=Dead+Cells%3A+Medley+of+Pain+Bundle+%28v35+%2B+5+DLCs+%2B+12+Bonus+OSTs%2C+MULTi13%29+%5BFitGirl+Repack%2C+Selective+Download+-+from+1.4+GB%5D&tr=udp%3A%2F%2Fopentor.net%3A6969&tr=udp%3A%2F%2Fopentracker.i2p.rocks%3A6969"
```

Proceso esperado:

```text
Magnet parsed
↓
Tracker contacted
↓
Peers discovered
↓
Handshake sent
↓
Handshake received
```

En concreto:

```
[*] Conectando ao tracker...
[+] Tracker: opentor.net:6969
[+] Connection ID: 2391868498765063790
[+] Intervalo: 1955 segundos
[+] Leechers: 6
[+] Seeders: 104

=============================================================
 Peers atopados : 110
=============================================================

[000] 5.144.121.70:63149
[001] 78.190.60.142:6881
[002] 81.39.90.190:6881
[003] 103.103.98.91:9558
[004] 149.100.144.165:48292
[005] 165.211.32.32:29043
[006] 218.201.30.96:6881
```
---

## Conceptos aprendidos

Durante el desarrollo de cTorrent se han aplicado conocimientos de:

### Redes

* TCP
* UDP
* DNS
* Sockets POSIX
* Network Byte Order

### Sistemas

* Gestión de memoria
* Serialización binaria
* Estructuras de datos
* Diseño modular
* Funcionamiento P2P

### Protocolos Distribuidos

* BitTorrent
* Tracker Protocol (BEP 15)
* Peer Discovery
* Handshake Negotiation

---

## Objetivo educativo

Este proyecto no pretende competir con clientes BitTorrent existentes.

El objetivo es comprender e implementar los mecanismos internos que permiten el funcionamiento de una red P2P moderna, desarrollando experiencia práctica en programación de sistemas y networking de bajo nivel.