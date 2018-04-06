#include "lib.h"

char staticArray[STATIC_ARRAY_SIZE][STATIC_BLOCK_SIZE];

void mkrndstr(char *block, size_t length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?! ";

    if (length) {
        if (block) {
            int l = (int) (sizeof(charset) -1);
            int key;
            for (int n = 0;n < length;n++) {
                key = rand() % l;
                block[n] = charset[key];
            }

            block[length] = '\0';
        }
    }
}

Array *createArray(int arraySize, int blockSize, bool isStatic){
    if(isStatic && arraySize > STATIC_ARRAY_SIZE) {
        printf("Maximum Static Array size is %d", STATIC_ARRAY_SIZE);
        return NULL;
    }
    if(isStatic && blockSize >= STATIC_BLOCK_SIZE) {
        printf("Maximum Static Block size is %d", STATIC_BLOCK_SIZE - 1);
        return NULL;
    }
    Array* blockArray = calloc(1, sizeof(Array));
    if(isStatic)
        blockArray->array = (char **) staticArray;
    else
        blockArray->array = calloc((size_t)arraySize, sizeof(char*));

    blockArray->arraySize = arraySize;
    blockArray->blockSize = blockSize;
    blockArray->isStatic = isStatic;
    return blockArray;
}

void addBlockAtIndex(Array* blockArray, int index){
    if(blockArray == NULL) return;
    if(index >= blockArray->arraySize || index < 0)
        printf("Index out of bounds");
    else if(!blockArray->isStatic && blockArray->array[index] != NULL)
        printf("Block is not empty");
    else {
        blockArray->array[index] = (calloc((size_t)blockArray->blockSize+1, sizeof(char)));
        mkrndstr(blockArray->array[index], (size_t) blockArray->blockSize);
    }
}

void deleteBlockAtIndex(Array* blockArray, int index){
    if(index >= blockArray->arraySize || index < 0){
        printf("Index out of bounds");
        return;
    }
    if(blockArray->array[index] == NULL) return;
    if(blockArray->isStatic)
        blockArray->array[index] = "";
    else{
        free(blockArray->array[index]);
        blockArray->array[index] = NULL;
    }
}

void deleteArray(Array* blockArray){
    if(blockArray == NULL) return;
    for (int i = 0; i < blockArray->arraySize; i++)
        deleteBlockAtIndex(blockArray, i);
    if(!blockArray->isStatic) {
        free(blockArray->array);
        blockArray->array = NULL;
    }
}


int blockToInt(char* block) {
    int result = 0;
    for(int i = 0; i < strlen(block); i ++)
        result += (int)block[i];
    return result;
}

char *findBlock(Array* blockArray, int searchValue) {
    char* closestBlock = NULL;
    int minDifference = INT_MAX;
    for(int i = 0; i < blockArray->arraySize; i++){
        char* block = blockArray->array[i];
        if(block != NULL){
            int difference = abs(blockToInt(block) - searchValue);
            if(minDifference >= difference && difference != 0){
                minDifference = difference;
                closestBlock = block;
            }
        }
    }
    return closestBlock;
}