CC = gcc
CFLAGS = -Wall -Wextra

all: compilation_dir client server

compilation_dir:
	mkdir -p compilation

client: compilation_dir src/client.c include/protocol.c
	$(CC) $(CFLAGS) -o compilation/client.out src/client.c include/protocol.c

server: compilation_dir src/server.c include/protocol.c
	$(CC) $(CFLAGS) -o compilation/server.out src/server.c include/protocol.c

clean:
	rm -rf compilation/*