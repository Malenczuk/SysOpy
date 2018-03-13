CC = gcc 
FLAGS = -std=c99 -Wall
LIB_NAME = lib
MAIN_SRC = main

compile: static shared dynamic

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

dynamic:
	# zbuduj biblioteke wspoldzielona .so
	$(CC) $(FLAGS) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) $(FLAGS) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_NAME).o

	# linkowanie
	$(CC) $(FLAGS) -Wl,-rpath=. -L. -o $(MAIN_SRC)_dynamic $(MAIN_SRC).c -ldl -D DLL

	make clean
	
clean:
	rm -f *~
	rm -f *.o
	rm -f *.a

clean_all: clean
	rm -f *.so
	rm -f $(MAIN_SRC)_dynamic
	rm -f $(MAIN_SRC)_shared
	rm -f $(MAIN_SRC)_static

test: compile
	echo 'static library' >> raport3a.txt
	./$(MAIN_SRC)_static 500000 512 dynamic f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_static 500000 512 static f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_static 500000 512 dynamic da 100000 cda 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_static 500000 512 static da 100000 cda 100000 | tee -a raport3a.txt
	echo 'shared library' >> raport3a.txt
	./$(MAIN_SRC)_shared 500000 512 dynamic f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_shared 500000 512 static f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_shared 500000 512 dynamic da 100000 cda 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_shared 500000 512 static da 100000 cda 100000 | tee -a raport3a.txt
	echo 'dynamic library' >> raport3a.txt
	./$(MAIN_SRC)_dynamic 500000 512 dynamic f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_dynamic 500000 512 static f 100 da 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_dynamic 500000 512 dynamic da 100000 cda 100000 | tee -a raport3a.txt
	./$(MAIN_SRC)_dynamic 500000 512 static da 100000 cda 100000 | tee -a raport3a.txt
	make clean_all