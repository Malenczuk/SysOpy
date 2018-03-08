#include "lib.h"

char staticArray[1000][1000];

Array *createArray(int arraySize, int blockSize, bool isStatic){
    if(arraySize <= 0) return NULL;
    Array* blockArray = calloc(1, sizeof(Array));
    if(isStatic){
        blockArray->array = (char **) staticArray;
    } else {
        blockArray->array = calloc((size_t)arraySize, sizeof(char*));
    }
    blockArray->arraySize = arraySize;
    blockArray->blockSize = blockSize;
    blockArray->isStatic = isStatic;
    return blockArray;
}

void addBlockAtIndex(Array* blockArray, char block[], int index){
    if(index >= blockArray->arraySize) return;
    if(strlen(block) >= blockArray->blockSize) return;
    if(blockArray->array[index] != NULL) return;
    if(blockArray->isStatic) blockArray->array[index] = block;
    else blockArray->array[index] = strcpy((calloc((size_t)blockArray->blockSize, sizeof(char))),block);
}

void deleteBlockAtIndex(Array* blockArray, int index){
    if(blockArray->array[index] == NULL) return;
    if(blockArray->isStatic) {
        blockArray->array[index] = "";
        return;
    }
    free(blockArray->array[index]);
}

void deleteArray(Array* blockArray){
    for (int i = 0; i < blockArray->arraySize; i++)
        deleteBlockAtIndex(blockArray, i);
    if(!blockArray->isStatic) free(blockArray->array);
    free(blockArray);
}

int blockToInt(char* block) {
    int result = 0;
    for(int i = 0; i < strlen(block); i ++) {
        result += (int)block[i];
    }
    return result;
}

char *findBlock(Array* blockArray, int searchValue) {
    char* closestBlock = NULL;
    int minDifference = INT_MAX;
    for(int i = 0; i < blockArray->arraySize; i++){
        char* block = blockArray->array[i];
        if(block != NULL){
            int difference = abs(blockToInt(block) - searchValue);
            if(minDifference > difference){
                minDifference = difference;
                closestBlock = block;
            }
        }
    }
    return closestBlock;
}
