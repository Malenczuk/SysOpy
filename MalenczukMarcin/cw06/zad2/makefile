CC = gcc
CFLAGS = -g -O2 -std=gnu99 -static -Wall -Wextra -Isrc -rdynamic -fomit-frame-pointer

all: clean client server

client:
	$(CC) $(CFLAGS) client.c -o client -lrt

server:
	$(CC) $(CFLAGS) server.c -o server -lrt


clean:
	rm -f client
	rm -f server
