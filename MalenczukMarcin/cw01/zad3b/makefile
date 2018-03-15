CC = gcc
FLAGS = -std=c99 -Wall
LIB_NAME = lib
MAIN_SRC = main
FILE ?= /dev/null

compile: static shared dynamic

static:
	# kompilacja zrodla
	$(CC) $(FLAGS) -c $(LIB_NAME).c -o $(LIB_NAME).o -O$(optim)

	# zbuduj biblioteke statyczna .a
	ar rcs lib$(LIB_NAME).a $(LIB_NAME).o

	# linkowanie
	$(CC) $(FLAGS) $(MAIN_SRC).c -I. lib$(LIB_NAME).a -o $(MAIN_SRC)_static -O$(optim)

	make clean

shared:
	# zbuduj biblioteke wspoldzielona .so
	$(CC) $(FLAGS) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o -O$(optim)
	$(CC) $(FLAGS) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_NAME).o -O$(optim)

	# linkowanie
	$(CC) $(FLAGS) -Wl,-rpath=. -L. lib$(LIB_NAME).so -o $(MAIN_SRC)_shared $(MAIN_SRC).c -O$(optim)

	make clean

dynamic:
	# zbuduj biblioteke wspoldzielona .so
	$(CC) $(FLAGS) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o -O$(optim)
	$(CC) $(FLAGS) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_NAME).o -O$(optim)

	# linkowanie
	$(CC) $(FLAGS) -Wl,-rpath=. -L. -o $(MAIN_SRC)_dynamic $(MAIN_SRC).c -ldl -D DLL -O$(optim)

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
	echo 'static library -O$(optim)' | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 dynamic find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 static find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 dynamic delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_static 500000 512 static delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	echo 'shared library -O$(optim)' | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 dynamic find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 static find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 dynamic delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_shared 500000 512 static delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	echo 'dynamic library -O$(optim)' | tee -a $(FILE)
	./$(MAIN_SRC)_dynamic 500000 512 dynamic find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_dynamic 500000 512 static find 100 delete_then_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_dynamic 500000 512 dynamic delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	./$(MAIN_SRC)_dynamic 500000 512 static delete_then_add 100000 delete_and_add 100000 | tee -a $(FILE)
	make clean_all

optim_test:
	make test optim=0
	make test optim=1
	make test optim=2
	make test optim=3
