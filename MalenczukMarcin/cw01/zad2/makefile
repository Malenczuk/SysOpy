CC = gcc 
FLAGS = -std=c99 -Wall
LIB_NAME = lib
MAIN_SRC = main
FILE ?= /dev/null

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

clean_all: clean
	rm -f *.so
	rm -f $(MAIN_SRC)_shared
	rm -f $(MAIN_SRC)_static

test: compile
	echo 'static library' >> $(FILE)
	./$(MAIN_SRC)_static 500000 512 dynamic find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 static find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 dynamic delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 static delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	echo 'shared library' >> $(FILE)
	./$(MAIN_SRC)_shared 500000 512 dynamic find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 static find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 dynamic delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 static delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	make clean_all