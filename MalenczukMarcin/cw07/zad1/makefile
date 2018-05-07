CC = gcc
CFLAGS = -g -O2 -std=gnu99 -Wall -lrt

all: barber creator

barber:
	$(CC) $(CFLAGS) hairdressers.c barber.c -o barber

creator:
	$(CC) $(CFLAGS) hairdressers.c creator.c -o creator


clean: $(patsubst %.c, %.clean, $(wildcard *.c))

%.clean:
	rm -f $(@:.clean=)
