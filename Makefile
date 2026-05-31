CC = gcc
CFLAGS = -Wall -Wextra

all: compilation_dir client server

compilation_dir:
	mkdir -p compilation

client: compilation_dir src/client.c
	$(CC) $(CFLAGS) -o compilation/client.out src/client.c

server: compilation_dir src/server.c
	$(CC) $(CFLAGS) -o compilation/server.out src/server.c

clean:
	rm -rf compilation/*