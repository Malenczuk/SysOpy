#include "sys.h"

int sysGenerateFile(char *filePath, int numberOfRecords, int recordSize){
    int file = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    int rnd = open("/dev/urandom", O_RDONLY);
    char *tmp = malloc(recordSize * sizeof(char));
    for (int i = 0; i < numberOfRecords; ++i){
        if(read(rnd, tmp, (size_t) recordSize * sizeof(char)) != recordSize) return 1;

        for(int j = 0; j < recordSize; j++)
            tmp[j] = (char) (abs(tmp[j]) % 25 + 65);
        tmp[recordSize-1] = 10;

        if(write(file, tmp, (size_t) recordSize * sizeof(char)) != recordSize) return 1;
    }
    close(file);
    close(rnd);
    free(tmp);
    return 0;
};

int sysCopyFile(char *sourcePath, char *targetPath, int numberOfRecords, int recordSize){
    int source = open(sourcePath, O_RDONLY);
    int target = open(targetPath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    char *tmp = malloc(recordSize * sizeof(char));

    for (int i = 0; i < numberOfRecords; i++){
        if(read(source, tmp, (size_t) recordSize * sizeof(char)) != recordSize) return 1;

        if(write(target, tmp, (size_t) recordSize * sizeof(char)) != recordSize) return 1;
    }
    close(source);
    close(target);
    free(tmp);
    return 0;
};

int sysSortFile(char *filePath, int numberOfRecords, int recordSize){
    int file = open(filePath, O_RDWR);
    char *r0 = malloc(recordSize * sizeof(char));
    char *r1 = malloc(recordSize * sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for(int i = 0; i < numberOfRecords; i++){
        lseek(file, i * offset, SEEK_SET);

        if(read(file, r0, (size_t) recordSize * sizeof(char)) != recordSize) return 1;

        for(j = i-1; j >= 0; j--){
            lseek(file, j * offset, SEEK_SET);
            if(read(file, r1, (size_t) recordSize * sizeof(char)) != recordSize) return 1;
            if(r0[0] >= r1[0]) break;
            if(write(file, r1, (size_t) recordSize * sizeof(char)) != recordSize) return 1;
        }
        lseek(file, (j+1) * offset, SEEK_SET);
        if(write(file, r0, (size_t) recordSize * sizeof(char)) != recordSize) return 1;
    }

    close(file);
    free(r0); free(r1);
    return 0;
};