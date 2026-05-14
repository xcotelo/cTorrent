CC = gcc
CFLAGS = -Wall -Wextra

all: client server

client: src/client.c
	$(CC) $(CFLAGS) -o compilation/client.out src/client.c

server: src/server.c
	$(CC) $(CFLAGS) -o compilation/server.out src/server.c

clean:
	rm -f client server
