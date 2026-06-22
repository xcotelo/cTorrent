CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

SRC = \
	src/main.c \
	src/tracker/tracker.c \
	src/peer/peer.c \
	src/protocol/handshake.c \
	src/protocol/protocol.c \
	src/torrent/magnet.c \
	src/peer/generate_peer_id.c

TARGET = compilation/rawtorrent

all: compilation_dir $(TARGET)

compilation_dir:
	mkdir -p compilation

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf compilation

.PHONY: all clean compilation_dir