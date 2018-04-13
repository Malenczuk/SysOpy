CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all: clean redirect main

redirect:
	$(CC) -Wall redirect.c -o redirect

main:
	$(CC) -Wall $(MAIN_SRC).c -o $(MAIN_SRC)

clean:
	rm redirect 2> /dev/null || echo > /dev/null
	rm $(MAIN_SRC) 2> /dev/null || echo > /dev/null
