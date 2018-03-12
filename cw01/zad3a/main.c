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

void * dll;

double timeDiff(clock_t start, clock_t end){
    return (double)(end -  start) / sysconf(_SC_CLK_TCK);
}

void printTime(clock_t rStartTime, struct tms tmsStartTime, clock_t rEndTime, struct tms tmsEndTime){
    printf("Real:   %lf   ", timeDiff(rStartTime, rEndTime));
    printf("User:   %lf   ", timeDiff(tmsStartTime.tms_utime, tmsEndTime.tms_utime));
    printf("System: %lf\n", timeDiff(tmsStartTime.tms_stime, tmsEndTime.tms_stime));
}

void fprintTime(FILE *f, clock_t rStartTime, struct tms tmsStartTime, clock_t rEndTime, struct tms tmsEndTime){
    fprintf(f, "Real:   %lf   ", timeDiff(rStartTime, rEndTime));
    fprintf(f, "User:   %lf   ", timeDiff(tmsStartTime.tms_utime, tmsEndTime.tms_utime));
    fprintf(f, "System: %lf\n", timeDiff(tmsStartTime.tms_stime, tmsEndTime.tms_stime));
}

int main(int argc, char *argv[]) {

    #ifdef DLL

    dll = dlopen("./liblib.so", RTLD_LAZY);
    typedef struct Array Array;
    Array * (*createArray)(int, int, bool) = dlsym(dll, "createArray");
    void (*deleteArray)(Array* ) = dlsym(dll, "deleteArray");
    void (*addBlockAtIndex)(Array*, int) = dlsym(dll,"addBlockAtIndex");
    void (*deleteBlockAtIndex)(Array*, int) = dlsym(dll, "deleteBlockAtIndex");
    char * (*findBlock)(Array*, int) = dlsym(dll, "findBlock");

    #endif

    int arraySize, blockSize;
    bool isStatic = false;
//    srand( time( NULL ) );

    if(argc < 4) {
        printf("Specify ArraySize, BlockSize and Memory allocation type\n");
        return (0);
    }

    arraySize = (int) strtol(argv[1], '\0', 10);
    blockSize = (int) strtol(argv[2], '\0', 10);
    if (strcmp(argv[3], "static") == 0)
        isStatic = true;
    else if (strcmp(argv[3], "dynamic") == 0)
        isStatic = false;
    else {
        printf("Use \"static\" or \"dynamic\" memory allocation");
        return (0);
    }

    FILE *f = fopen("raport3a.txt", "a");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    clock_t rTime[6] = {0, 0, 0, 0, 0, 0};
    struct tms* tmsTime[6];
    for (int i = 0; i < 6; i++) {
        tmsTime[i] = malloc(sizeof(struct tms*));
    }
    int currentTime = 0;

    printf("ArraySize: %d   BlockSize: %d   Allocation: %s\n", arraySize, blockSize, argv[3]);
    fprintf(f, "ArraySize: %d   BlockSize: %d   Allocation: %s\n", arraySize, blockSize, argv[3]);

    rTime[currentTime] = times(tmsTime[currentTime]);
    currentTime++;

    Array *array = createArray(arraySize, blockSize, isStatic);
    for (int i = 0; i < arraySize; i++) {
        addBlockAtIndex(array, i);
    }

    rTime[currentTime] = times(tmsTime[currentTime]);
    currentTime++;

    printf("Creating array:\n");
    printTime(rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);
    fprintf(f, "Creating array:\n");
    fprintTime(f, rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);


    for(int i = 4; i + 1 < argc && i < 8; i+=2){
        int startIndex = 0;
        int arg = (int) strtol(argv[i+1], '\0', 10);
        rTime[currentTime] = times(tmsTime[currentTime]);
        currentTime++;

        if(strcmp(argv[i], "f") == 0){
            if(arg < 0 && arg >= arraySize) {
                printf("Index out of bounds");
            }
            printf("Finding block: \n");
            fprintf(f,"Finding block: \n");
            findBlock(array, arg);
        }
        else if(strcmp(argv[i], "da") == 0){
            if(startIndex < 0 && startIndex >= arraySize)
                printf("Index out of bounds");
            else{
                printf("Deleting then Adding %d Blocks: \n", arg);
                fprintf(f, "Deleting then Adding %d Blocks: \n", arg);
                for(int i = 0; (i + startIndex) < arraySize && i < arg; i++) {
                    deleteBlockAtIndex(array, i);
                }
                for(int i = 0; (i + startIndex) < arraySize && i < arg; i++) {
                    addBlockAtIndex(array, i);
                }
            }
        }
        else if(strcmp(argv[i], "cda") == 0){
            if(startIndex < 0 && startIndex >= arraySize)
                printf("Index out of bounds");
            else{
                printf("Cross Deleting and Adding %d Blocks: \n", arg);
                fprintf(f, "Cross Deleting and Adding %d Blocks: \n", arg);
                for(int i = 0; (i + startIndex) < arraySize && i < arg; i++) {
                    deleteBlockAtIndex(array, i);
                    addBlockAtIndex(array, i);
                }
            }
        }
        else {
            printf("Wrong operation\n");
            return(0);
        }

        rTime[currentTime] = times(tmsTime[currentTime]);
        currentTime++;

        printTime(rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);
        fprintTime(f, rTime[currentTime-2], *tmsTime[currentTime-2], rTime[currentTime-1], *tmsTime[currentTime-1]);
    }
    printf("\n");
    fprintf(f, "\n");
    deleteArray(array);

    fclose(f);


#ifdef DLL
    dlclose(dll);
#endif

    return(0);
}