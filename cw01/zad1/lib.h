#ifndef INC_1_LIB_H
#define INC_1_LIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include <stdbool.h>

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

#endif //INC_1_LIB_H
