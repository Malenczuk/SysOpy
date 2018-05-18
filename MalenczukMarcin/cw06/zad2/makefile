CC = gcc
CFLAGS = -g -O2 -std=gnu99 -Wall

all: clean client server

client:
	$(CC) $(CFLAGS) client.c -o client -lrt

server:
	$(CC) $(CFLAGS) server.c -o server -lrt


clean:
	rm -f client
	rm -f server
