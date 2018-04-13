#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFF_SIZE 256

int main(int argc, char *argv[]) {
    char buffer[BUFF_SIZE];
    if (argc < 3) return 1;
    int file = open(argv[2], O_CREAT | O_RDWR | (strstr(argv[1], ">>") ? O_APPEND : O_TRUNC), S_IRUSR | S_IWUSR);
    size_t n;
    while((n = (size_t) read(0, buffer, BUFF_SIZE)) != 0){
        write(file, buffer, n);
        write(1, buffer, n);
    }
    return 0;
}