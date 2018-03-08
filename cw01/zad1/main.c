#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib.h"

int main() {
    Array* blockArray = createArray(2, 30, false);
    addBlockAtIndex(blockArray, "xD", 0);
    addBlockAtIndex(blockArray, "ALA", 1);
    printf("%s\n", findBlock(blockArray, 200));
    deleteBlockAtIndex(blockArray, 1);
    printf("%s\n", findBlock(blockArray, 188));
    deleteArray(blockArray);
    return 0;
}