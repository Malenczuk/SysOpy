CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all: clean slave master main

slave:
	$(CC) -Wall slave.c -o slave

master:
	$(CC) -Wall master.c -o master

main:
	$(CC) -Wall $(MAIN_SRC).c -o $(MAIN_SRC)


clean:
	rm $(MAIN_SRC) 2> /dev/null || echo > /dev/null
	rm master 2> /dev/null || echo > /dev/null
	rm slave 2> /dev/null || echo > /dev/null
