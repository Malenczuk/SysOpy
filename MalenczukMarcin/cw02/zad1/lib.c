#include <_G_config.h>
#include "lib.h"

int generateFile(char *filePath, int numberOfRecords, int recordSize){
    FILE *file = fopen(filePath, "w+");
    if(!file){
        printf("Error while opening %s.\n", filePath);
        return 1;
    }

    FILE *rnd = fopen("/dev/urandom", "r");
    if(!rnd){
        printf("Error while opening /dev/urandom.\n");
        return 1;
    }

    char *tmp = malloc(recordSize * sizeof(char));
    for (int i = 0; i < numberOfRecords; ++i){
        if(fread(tmp, sizeof(char), (size_t) recordSize, rnd) != recordSize) {
            printf("Error while reading /dev/urandom.\n");
            return 1;
        }

        for(int j = 0; j < recordSize; j++)
            tmp[j] = (char) (abs(tmp[j]) % 25 + 65);
        tmp[recordSize-1] = 10;

        if(fwrite(tmp, sizeof(char), (size_t) recordSize, file) != recordSize) {
            printf("Error while writing to %s.\n", filePath);
            return 1;
        }
    }
    fclose(file);
    fclose(rnd);
    free(tmp);
    return 0;
};

int copyFile(char *sourcePath, char *targetPath, int numberOfRecords, int recordSize){
    FILE *source = fopen(sourcePath, "r");
    if(!source){
        printf("Error while opening %s.\n", sourcePath);
        return 1;
    }
    FILE *target = fopen(targetPath, "w+");
    if(!target){
        printf("Error while opening %s.\n", targetPath);
        return 1;
    }
    char *tmp = malloc(recordSize * sizeof(char));

    for (int i = 0; i < numberOfRecords; i++){
        if(fread(tmp, sizeof(char), (size_t) recordSize, source) != recordSize) return 1;

        if(fwrite(tmp, sizeof(char), (size_t) recordSize, target) != recordSize) return 1;
    }
    fclose(source);
    fclose(target);
    free(tmp);
    return 0;
};

int sortFile(char *filePath, int numberOfRecords, int recordSize){
    FILE *file = fopen(filePath, "r+");
    if(!file){
        printf("Error while opening %s.\n", filePath);
        return 1;
    }
    char *r0 = malloc(recordSize * sizeof(char));
    char *r1 = malloc(recordSize * sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for(int i = 0; i < numberOfRecords; i++){
        fseek(file, i * offset, 0);

        if(fread(r0, sizeof(char), (size_t) recordSize, file) != recordSize) return 1;

        for(j = i-1; j >= 0; j--){
            fseek(file, j * offset, 0);
            if(fread(r1, sizeof(char), (size_t) recordSize, file) != recordSize) return 1;
            if(r0[0] >= r1[0]) break;
            if(fwrite(r1, sizeof(char), (size_t) recordSize, file) != recordSize) return 1;
        }
        fseek(file, (j+1) * offset, 0);
        if(fwrite(r0, sizeof(char), (size_t) recordSize, file) != recordSize) return 1;
    }

    fclose(file);
    free(r0); free(r1);
    return 0;
};