CC = gcc
FLAGS = -std=c99 -Wall
MAIN_SRC = main
OPTIM ?= -O2
ITER ?= 1
FILE ?= /dev/null

main:
	$(CC) $(FLAGS) lib.c sys.c $(MAIN_SRC).c -o $(MAIN_SRC) $(OPTIM)
	
	make clean

clean:
	rm -f *.o
	rm -f *.a

clean_all: clean
	rm $(MAIN_SRC)

test: main
	./main -F txt.txt -n 5000000 -s 4 --gen --lib
	./main -F txt.txt -n 5000000 -s 4 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000000 -s 4 --copy txt2.txt --sys | tee -a $(FILE)
	./main -F txt.txt -n 10000000 -s 4 --gen --lib
	./main -F txt.txt -n 10000000 -s 4 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 10000000 -s 4 --copy txt2.txt --sys | tee -a $(FILE)

	./main -F txt.txt -n 5000000 -s 512 --gen --lib
	./main -F txt.txt -n 5000000 -s 512 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000000 -s 512 --copy txt2.txt --sys | tee -a $(FILE)
	./main -F txt.txt -n 10000000 -s 512 --gen --lib
	./main -F txt.txt -n 10000000 -s 512 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 10000000 -s 512 --copy txt2.txt --sys | tee -a $(FILE)

	./main -F txt.txt -n 500000 -s 4096 --gen --lib
	./main -F txt.txt -n 500000 -s 4096 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 500000 -s 4096 --copy txt2.txt --sys | tee -a $(FILE)
	./main -F txt.txt -n 1000000 -s 4096 --gen --lib
	./main -F txt.txt -n 1000000 -s 4096 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 1000000 -s 4096 --copy txt2.txt --sys | tee -a $(FILE)

	./main -F txt.txt -n 100000 -s 8192 --gen --lib
	./main -F txt.txt -n 100000 -s 8192 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 100000 -s 8192 --copy txt2.txt --sys | tee -a $(FILE)
	./main -F txt.txt -n 500000 -s 8192 --gen --lib
	./main -F txt.txt -n 500000 -s 8192 --copy txt2.txt --lib | tee -a $(FILE)
	./main -F txt.txt -n 500000 -s 8192 --copy txt2.txt --sys | tee -a $(FILE)

	./main -F txt.txt -n 2000 -s 4 --gen --lib
	./main -F txt.txt -n 2000 -s 4 --copy txt2.txt --lib
	./main -F txt2.txt -n 2000 -s 4 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 2000 -s 4 --copy txt2.txt --sys
	./main -F txt2.txt -n 2000 -s 4 --sort --sys | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 4 --gen --lib
	./main -F txt.txt -n 5000 -s 4 --copy txt2.txt --lib
	./main -F txt2.txt -n 5000 -s 4 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 4 --copy txt2.txt --sys
	./main -F txt2.txt -n 5000 -s 4 --sort --sys | tee -a $(FILE)

	./main -F txt.txt -n 2000 -s 512 --gen --lib
	./main -F txt.txt -n 2000 -s 512 --copy txt2.txt --lib
	./main -F txt2.txt -n 2000 -s 512 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 2000 -s 512 --copy txt2.txt --sys
	./main -F txt2.txt -n 2000 -s 512 --sort --sys | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 512 --gen --lib
	./main -F txt.txt -n 5000 -s 512 --copy txt2.txt --lib
	./main -F txt2.txt -n 5000 -s 512 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 512 --copy txt2.txt --sys
	./main -F txt2.txt -n 5000 -s 512 --sort --sys | tee -a $(FILE)

	./main -F txt.txt -n 2000 -s 4096 --gen --lib
	./main -F txt.txt -n 2000 -s 4096 --copy txt2.txt --lib
	./main -F txt2.txt -n 2000 -s 4096 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 2000 -s 4096 --copy txt2.txt --sys
	./main -F txt2.txt -n 2000 -s 4096 --sort --sys | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 4096 --gen --lib
	./main -F txt.txt -n 5000 -s 4096 --copy txt2.txt --lib
	./main -F txt2.txt -n 5000 -s 4096 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 4096 --copy txt2.txt --sys
	./main -F txt2.txt -n 5000 -s 4096 --sort --sys | tee -a $(FILE)

	./main -F txt.txt -n 2000 -s 8192 --gen --lib
	./main -F txt.txt -n 2000 -s 8192 --copy txt2.txt --lib
	./main -F txt2.txt -n 2000 -s 8192 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 2000 -s 8192 --copy txt2.txt --sys
	./main -F txt2.txt -n 2000 -s 8192 --sort --sys | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 8192 --gen --lib
	./main -F txt.txt -n 5000 -s 8192 --copy txt2.txt --lib
	./main -F txt2.txt -n 5000 -s 8192 --sort --lib | tee -a $(FILE)
	./main -F txt.txt -n 5000 -s 8192 --copy txt2.txt --lib
	./main -F txt2.txt -n 5000 -s 8192 --sort --sys | tee -a $(FILE)

	make clean_all