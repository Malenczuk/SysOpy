CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all: clean
	make main

main:
	$(CC) -Wall $(MAIN_SRC).c -o $(MAIN_SRC)


clean:
	rm $(MAIN_SRC)
