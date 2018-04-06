CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all:
	$(CC) -Wall $(MAIN_SRC).c -o $(MAIN_SRC)
	$(CC) -Wall child.c -o child

clean:
	rm $(MAIN_SRC)
	rm child
