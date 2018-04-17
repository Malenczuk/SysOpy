CC = gcc
FLAGS = -std=gnu99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all: clean slave master main

slave:
	$(CC) $(FLAGS) slave.c -o slave

master:
	$(CC) $(FLAGS) master.c -o master

main:
	$(CC) $(FLAGS) $(MAIN_SRC).c -o $(MAIN_SRC)


clean:
	rm $(MAIN_SRC) 2> /dev/null || echo > /dev/null
	rm master 2> /dev/null || echo > /dev/null
	rm slave 2> /dev/null || echo > /dev/null
