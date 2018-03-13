#include <dlfcn.h>

#ifndef DLL
#include "lib.h"
#endif

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef DLL
void * dll;
typedef struct {
    char** array;
    int arraySize;
    int blockSize;
    bool isStatic;
} Array;
#endif

double timeDiff(clock_t start, clock_t end){
    return (double)(end -  start) / sysconf(_SC_CLK_TCK);
}

void printTime(clock_t rStartTime, struct tms tmsStartTime, clock_t rEndTime, struct tms tmsEndTime){
    printf("Real:   %.2lf s   ", timeDiff(rStartTime, rEndTime));
    printf("User:   %.2lf s   ", timeDiff(tmsStartTime.tms_utime, tmsEndTime.tms_utime));
    printf("System: %.2lf s\n", timeDiff(tmsStartTime.tms_stime, tmsEndTime.tms_stime));
}

void crossDeleteAndAdd(Array *array, int startIndex, int arg){
    #ifdef DLL
    void (*deleteBlockAtIndex)(Array*, int) = dlsym(dll, "deleteBlockAtIndex");
    void (*addBlockAtIndex)(Array*, int) = dlsym(dll,"addBlockAtIndex");
    #endif
    if(startIndex < 0 && startIndex >= array->arraySize)
        printf("Index out of bounds");
    else{
        printf("Cross Deleting and Adding %d Blocks: \n", arg);
        for(int i = 0; (i + startIndex) < array->arraySize && i < arg; i++) {
            deleteBlockAtIndex(array, i);
            addBlockAtIndex(array, i);
        }
    }

}

void deleteAndAdd(Array *array, int startIndex, int arg){
    #ifdef DLL
    void (*deleteBlockAtIndex)(Array*, int) = dlsym(dll, "deleteBlockAtIndex");
    void (*addBlockAtIndex)(Array*, int) = dlsym(dll,"addBlockAtIndex");
    #endif
    if(startIndex < 0 && startIndex >= array->arraySize)
        printf("Index out of bounds");
    else{
        printf("Deleting then Adding %d Blocks: \n", arg);
        for(int i = 0; (i + startIndex) < array->arraySize && i < arg; i++) {
            deleteBlockAtIndex(array, i);
        }
        for(int i = 0; (i + startIndex) < array->arraySize && i < arg; i++) {
            addBlockAtIndex(array, i);
        }
    }
}

void find(Array *array, int arg){
    #ifdef DLL
    char * (*findBlock)(Array*, int) = dlsym(dll, "findBlock");
    #endif
    if(arg < 0 && arg >= array->arraySize) {
        printf("Index out of bounds");
    }
    printf("Finding block: \n");
    findBlock(array, arg);
}

int execute(Array *array, char *command, int arg){
    if(strcmp(command, "f") == 0){
        find(array, arg);
    }
    else if(strcmp(command, "da") == 0){
        deleteAndAdd(array, 0, arg);
    }
    else if(strcmp(command, "cda") == 0){
        crossDeleteAndAdd(array, 0, arg);
    }
    else {
        printf("Wrong operation\n");
        return(1);
    }
    return(0);
}

void fillArray(Array *array){
    #ifdef DLL
    void (*addBlockAtIndex)(Array*, int) = dlsym(dll,"addBlockAtIndex");
    #endif
    for (int i = 0; i < array->arraySize; i++) {
        addBlockAtIndex(array, i);
    }
}

int checkArg(int arraySize, int blockSize){
    if(arraySize <= 0) {
        printf("Array Size must be positive");
        return 1;
    }
    if(blockSize <= 0) {
        printf("Block Size must be positive");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    #ifdef DLL
    dll = dlopen("./liblib.so", RTLD_LAZY);

    Array * (*createArray)(int, int, bool) = dlsym(dll, "createArray");
    void (*deleteArray)(Array* ) = dlsym(dll, "deleteArray");
    #endif

    int arraySize, blockSize;
    bool isStatic = false;
//    srand( time( NULL ) );

    if(argc < 4) {
        printf("Specify ArraySize, BlockSize and Memory allocation type(\"static\" or \"dynamic\")\n");
        exit(1);
    }

    arraySize = (int) strtol(argv[1], '\0', 10);
    blockSize = (int) strtol(argv[2], '\0', 10);
    if (strcmp(argv[3], "static") == 0)
        isStatic = true;
    else if (strcmp(argv[3], "dynamic") == 0)
        isStatic = false;
    else {
        printf("Use \"static\" or \"dynamic\" memory allocation");
        exit(1);
    }
    if(checkArg(arraySize, blockSize))
        exit(1);

    clock_t rTime[6] = {0, 0, 0, 0, 0, 0};
    struct tms* tmsTime[6];
    for (int i = 0; i < 6; i++) {
        tmsTime[i] = malloc(sizeof(struct tms*));
    }
    int currentTime = 0;

    printf("ArraySize: %d   BlockSize: %d   Allocation: %s\n", arraySize, blockSize, argv[3]);

    rTime[currentTime] = times(tmsTime[currentTime]);
    currentTime++;

    Array *array = createArray(arraySize, blockSize, isStatic);
    if(!array){
        printf("Error creating Array");
        exit(1);
    }
    fillArray(array);

    rTime[currentTime] = times(tmsTime[currentTime]);
    currentTime++;

    printf("Creating array:\n");
    printTime(rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);

    for(int i = 4; i + 1 < argc && i < 8; i+=2){
        int arg = (int) strtol(argv[i+1], '\0', 10);
        rTime[currentTime] = times(tmsTime[currentTime]);
        currentTime++;
        
        if(execute(array, argv[i], arg)){
            printf("Wrong command\n");
        }
    
        rTime[currentTime] = times(tmsTime[currentTime]);
        currentTime++;
        printTime(rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);
    }
    printf("\n");
    deleteArray(array);

#ifdef DLL
    dlclose(dll);
#endif

    return(0);
}
