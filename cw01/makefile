CC = gcc -Wall
LIB_NAME = lib


static:
	# kompilacja zrodla
	$(CC) -c $(LIB_NAME).c -o $(LIB_NAME).o

	# zbuduj biblioteke statyczna .a
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

	# czyscimy
	make clean

shared:

	# zbuduj biblioteke wspoldziselona .so
	$(CC) -fPIC -c $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) -fPIC -shared -o $(LIB_NAME).so $(LIB_NAME).o

	# czyscimy
	make clean

clean:
	rm -f *.o