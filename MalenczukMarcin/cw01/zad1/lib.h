#ifndef zad2_LIB_H
#define zad2_LIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include <stdbool.h>
#include <limits.h>

#define STATIC_ARRAY_SIZE 1000000
#define STATIC_BLOCK_SIZE 1001


typedef struct {
    char** array;
    int arraySize;
    int blockSize;
    bool isStatic;
} Array;

extern char staticArray[STATIC_ARRAY_SIZE][STATIC_BLOCK_SIZE];

extern void mkrndstr(char *block, size_t length);
extern Array *createArray(int arraySize, int blockSize, bool isStatic);
extern void deleteArray(Array* blockArray);
extern void addBlockAtIndex(Array* blockArray, int index);
extern void deleteBlockAtIndex(Array* blockArray, int index);
extern int blockToInt(char* block);
extern char *findBlock(Array* blockArray, int searchValue);

#endif //zad2_LIB_H
