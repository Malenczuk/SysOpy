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
	$(CC) $(FLAGS) $(MAIN_SRC).c -L. -l $(LIB_NAME) -o $(MAIN_SRC)_static

shared:
	# zbuduj biblioteke wspoldzielona .so
	$(CC) $(FLAGS) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) $(FLAGS) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_NAME).o

	# linkowanie
	$(CC) -Wl,-rpath=. -L. -o $(MAIN_SRC)_shared $(MAIN_SRC).c -l$(LIB_NAME)


clean:
	rm -f *~
	rm -f *.o
	rm -f *.a
	rm -f *.so

clean_all: clean
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
	clean_all