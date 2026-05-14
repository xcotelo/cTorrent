# Proyecto: Cliente BitTorrent en C (roadmap completo)

## Objetivo final

Construir un cliente BitTorrent funcional desde cero en C centrado en networking y sistemas:

* Sockets TCP/UDP
* Magnet links
* Descubrimiento de peers
* DHT
* Descarga por piezas
* Ensamblado de archivos
* Concurrencia
* Logging y métricas

La idea no es clonar una web de torrents. El objetivo es comprender y construir la infraestructura de red que existe por debajo.

Duración estimada:

* MVP: 4–6 semanas
* Versión sólida: 3–4 meses
* Avanzado: 6+ meses

---

# Stack

Lenguaje principal:

* C

Herramientas:

* gcc
* gdb
* make
* CMake
* Git
* VS Code

Librerías:

Necesarias:

* pthread
* OpenSSL (SHA1)

Opcionales:

* ncurses
* sqlite

Python (solo opcional):

* scripts de pruebas
* visualización
* benchmarks

---

Primer programa:

Servidor:

```c
socket()
bind()
listen()
accept()
```

Cliente:

```c
socket()
connect()
send()
recv()
```

Prueba:

Terminal 1:

```bash
./server
```

Terminal 2:

```bash
./client
```

Resultado:

```text
Cliente:
Hola servidor

Servidor:
Mensaje recibido
```

Parece trivial, pero aquí aprendes:

* sockets
* puertos
* TCP
* buffers
* `sockaddr_in`

Eso será la base de todo lo demás.

---

**Semana 2: mini chat concurrente**

Ahora cambia a:

```text
Cliente A
      \
       Servidor
      /
Cliente B
```

Añade:

* `pthread`
* múltiples clientes
* logs
* desconexiones
* timestamps

Aprendes:

* concurrencia
* mutex
* race conditions
* gestión de clientes

Esto ya se parece a un peer BitTorrent pequeño.

---

**Semana 3: parsing binario**

BitTorrent habla con mensajes binarios, no con texto.

Haz un protocolo propio:

```text
| TYPE | LENGTH | PAYLOAD |
```

Ejemplo:

```text
01|5|hola
```

Funciones:

```c
serialize_message()
parse_message()
```

Aprenderás:

* endianess
* estructuras
* buffers
* protocolos

---

**Semana 4: magnet links**

Ahora sí entras en el mundo torrent.

Entrada:

```text
magnet:?xt=urn:btih:ABC123
```

Tu código:

```c
parse_magnet()
```

Salida:

```text
INFO HASH:
ABC123
```

Sin red todavía.

Solo entender el formato.

---

**Semana 5–6: handshake BitTorrent**

Aquí ya conectas con peers reales.

Implementas:

```c
build_handshake()
parse_handshake()
```

El flujo:

```text
Tu cliente
      ↓
Peer público
      ↓
Handshake correcto
```

Si consigues esto, ya estás dentro de la red.

---

**Semana 7–8: descargar una pieza**

Objetivo:

No descargar un archivo completo.

Solo:

```text
pieza 0
```

y guardarla:

```c
fwrite()
```

Si descargas una pieza real:

```text
Has descargado bytes desde un peer BitTorrent
```

ya has cruzado el punto difícil.

---

Después:

* múltiples peers
* scheduler
* verificación SHA1
* DHT
* streaming

---

Si me preguntas qué haría hoy mismo, literalmente sería:

1. Crear la carpeta del proyecto.
2. Configurar VS Code.
3. Hacer un servidor TCP de 50 líneas.
4. Hacer un cliente TCP de 50 líneas.
5. Ver el tráfico con Wireshark.

Y no avanzaría a la siguiente fase hasta entender exactamente qué hacen:

```c
socket()
bind()
listen()
accept()
connect()
send()
recv()
```

Porque el 80% del proyecto descansa sobre esas llamadas.


---

# Estructura del proyecto (VS Code)

project/
│
├── .vscode/
│   ├── tasks.json
│   ├── launch.json
│
├── include/
│   ├── peer.h
│   ├── logger.h
│   ├── magnet.h
│   ├── torrent.h
│   ├── dht.h
│   ├── scheduler.h
│
├── src/
│   ├── main.c
│   │
│   ├── network/
│   │      socket.c
│   │      peer.c
│   │
│   ├── protocol/
│   │      handshake.c
│   │      messages.c
│   │
│   ├── torrent/
│   │      magnet.c
│   │      metadata.c
│   │      pieces.c
│   │
│   ├── dht/
│   │      dht.c
│   │      routing.c
│   │
│   ├── scheduler/
│   │      scheduler.c
│   │
│   ├── utils/
│          logger.c
│          hash.c
│
├── downloads/
├── logs/
├── tests/
├── Makefile
├── CMakeLists.txt
└── README.md

--- 

# Qué debes poder explicar al terminar

* TCP vs UDP
* Handshake BitTorrent
* DHT
* Kademlia
* SHA1
* descarga por piezas
* concurrencia
* scheduler
* sockets
* sincronización
* gestión de memoria
* diseño de protocolos
