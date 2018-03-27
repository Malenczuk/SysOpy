CC = gcc
FLAGS = -std=c11 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all: main
	$(CC) $(FLAGS) cpu_destroyer.c -o cpu_destroyer $(OPTIM)
	$(CC) $(FLAGS) memory_destroyer.c -o memory_destroyer $(OPTIM)
main:
	$(CC) $(FLAGS) $(MAIN_SRC).c -o $(MAIN_SRC) $(OPTIM)

clean:
	rm $(MAIN_SRC)
	rm memory_destroyer	
	rm cpu_destroyer

test: all
	./main txt.txt 1000 10000
	./main txt.txt 1000 10
	./main txt.txt 10   10000

	