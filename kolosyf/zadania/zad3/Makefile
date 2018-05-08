CFLAGS = -Wall
CC = gcc

main: main.o

clean:
	rm -f main.o main

test:  main
	@echo -e "\nTESTING\n"
	./main 5 
	./main 7
	./main 100 | wc -l
	./main 200 | wc -l
all: main test
