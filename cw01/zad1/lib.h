#ifndef zad1_LIB_H
#define zad1_LIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
    char** array;
    int arraySize;
    int blockSize;
    bool isStatic;
} Array;

extern char staticArray[1000][1000];

Array *createArray(int arraySize, int blockSize, bool isStatic);
void deleteArray(Array* blockArray);
void addBlockAtIndex(Array* blockArray, char* block, int index);
void deleteBlockAtIndex(Array* blockArray, int index);
int blockToInt(char* block);
char *findBlock(Array* blockArray, int searchValue);

#endif //zad1_LIB_H
