CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

main:
	$(CC) $(FLAGS) $(MAIN_SRC).c -o $(MAIN_SRC) $(OPTIM)

clean:
	rm $(MAIN_SRC)	

test:
	./main txt.txt