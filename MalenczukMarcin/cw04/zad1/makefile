CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all:
	make main
	make main2

main:
	$(CC) -Wall $(MAIN_SRC).c -o $(MAIN_SRC)

main2:
	$(CC) -Wall $(MAIN_SRC)2.c -o $(MAIN_SRC)2

clean:
	rm $(MAIN_SRC)
	rm $(MAIN_SRC)2