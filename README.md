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

## Peer Discovery

Una vez procesado el **Magnet Link**, el cliente establece comunicación con un tracker UDP compatible con **BEP-15** para solicitar la lista de participantes (*swarm*) asociada al `info_hash`.

Tras completar correctamente el intercambio **CONNECT** y **ANNOUNCE**, el tracker devuelve una **Compact Peer List**, donde cada entrada está formada por una dirección IPv4 y un puerto TCP.

Cada peer recibido se almacena internamente mediante la siguiente estructura:

```c
typedef struct {
    uint32_t ip;
    uint16_t port;
} TrackerPeer;
```

Durante esta fase el cliente:

* Resuelve el nombre DNS del tracker.
* Implementa manualmente el protocolo **UDP Tracker (BEP-15)**.
* Gestiona `connection_id` temporales.
* Implementa reintentos con *exponential backoff* ante pérdidas de paquetes UDP.
* Analiza la respuesta compacta enviada por el tracker.
* Construye una lista indexada de todos los peers anunciados.

Ejemplo de salida:

```text
=============================================================
 Peers encontrados : 137
=============================================================

[000] 1.242.244.122:64989
[001] 27.89.30.189:17883
[002] 49.66.154.112:20073
...
[136] 5.180.208.204:8412
```

Esta fase representa el proceso inicial de descubrimiento de nodos dentro de la red BitTorrent y constituye el punto de partida para el establecimiento de conexiones TCP con otros participantes del *swarm*.

---

## Verificación de peers

Una vez obtenida la lista de peers proporcionada por el tracker, **cTorrent** intenta establecer una conexión TCP con cada uno de ellos de forma independiente.

El objetivo de esta etapa es determinar qué peers se encuentran realmente accesibles en el momento de la ejecución. Aunque un tracker anuncie cientos de direcciones, no todas corresponden necesariamente a clientes activos o alcanzables: algunos peers pueden haberse desconectado, estar protegidos por NAT o firewall, o simplemente no responder dentro del tiempo establecido.

Para cada peer el cliente:

* Abre una conexión TCP.
* Establece un tiempo máximo de espera (*connection timeout*).
* Verifica que el socket alcance el estado **CONNECTED**.
* Registra el resultado de la operación.

La salida indica, para cada peer descubierto, si la conexión pudo establecerse correctamente:

```text
[*] Probando peers...

[*] [001] 27.89.30.189:17883 ........ OK
[*] [002] 49.66.154.112:20073 ........ OK
[*] [006] 81.39.90.190:6881 ........ FAIL
[*] [020] 146.70.182.5:47434 ........ OK
...
```

Los peers marcados como **OK** representan candidatos válidos para iniciar el protocolo **Peer Wire**, mientras que aquellos marcados como **FAIL** son descartados por no aceptar conexiones o no responder dentro del tiempo límite.

Esta comprobación permite filtrar la lista anunciada por el tracker y conservar únicamente los nodos potencialmente utilizables para las siguientes fases del cliente.

---

## BitTorrent Handshake

Tras identificar un peer accesible mediante una conexión TCP satisfactoria, el cliente inicia el **BitTorrent Handshake**, primer intercambio obligatorio definido por el protocolo **Peer Wire**.

El *handshake* permite verificar que ambos extremos pertenecen al mismo *swarm* y negocian el mismo protocolo antes de intercambiar cualquier otra información.

El mensaje tiene un tamaño fijo de **68 bytes** y contiene la siguiente estructura:

```text
<pstrlen>
<pstr>
<reserved>
<info_hash>
<peer_id>
```

Durante esta fase el cliente verifica:

* Que el peer implementa el protocolo BitTorrent.
* Que el `info_hash` coincide con el torrent solicitado.
* La identidad (`peer_id`) del nodo remoto.
* Que la conexión puede utilizarse para el intercambio posterior de mensajes del protocolo **Peer Wire**.

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