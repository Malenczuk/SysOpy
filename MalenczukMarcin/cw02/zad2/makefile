CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

static:
	$(CC) $(FLAGS) $(MAIN_SRC).c -o $(MAIN_SRC)_static $(OPTIM)
	
	make clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
	
clean_all:
	rm $(MAIN_SRC)_static		