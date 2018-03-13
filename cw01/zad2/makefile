CC = gcc 
FLAGS = -std=c99 -Wall
LIB_NAME = lib
MAIN_SRC = main

compile: static shared

static:
	# kompilacja zrodla
	$(CC) $(FLAGS) -c $(LIB_NAME).c -o $(LIB_NAME).o

	# zbuduj biblioteke statyczna .a
	ar rcs lib$(LIB_NAME).a $(LIB_NAME).o

	# linkowanie
	$(CC) $(FLAGS) $(MAIN_SRC).c -I. lib$(LIB_NAME).a -o $(MAIN_SRC)_static

	make clean

shared:
	# zbuduj biblioteke wspoldzielona .so
	$(CC) $(FLAGS) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) $(FLAGS) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_NAME).o

	# linkowanie
	$(CC) $(FLAGS) -Wl,-rpath=. -L. lib$(LIB_NAME).so -o $(MAIN_SRC)_shared $(MAIN_SRC).c 

	make clean

clean:
	rm -f *~
	rm -f *.o
	rm -f *.a
	rm -f *.so

clean_all: clean
	rm -f $(MAIN_SRC)_shared
	rm -f $(MAIN_SRC)_static

test: compile
	echo 'static library' >> raport2.txt
	./$(MAIN_SRC)_static 500000 512 dynamic f 100 da 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_static 500000 512 static f 100 da 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_static 500000 512 dynamic da 100000 cda 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_static 500000 512 static da 100000 cda 100000 | tee -a raport2.txt
	echo 'shared library' >> raport2.txt
	./$(MAIN_SRC)_shared 500000 512 dynamic f 100 da 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_shared 500000 512 static f 100 da 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_shared 500000 512 dynamic da 100000 cda 100000 | tee -a raport2.txt
	./$(MAIN_SRC)_shared 500000 512 static da 100000 cda 100000 | tee -a raport2.txt
	make clean_all