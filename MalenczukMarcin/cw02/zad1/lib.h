#ifndef ZAD1_LIB_H
#define ZAD1_LIB_H

#include <stdio.h>
#include <stdlib.h>

int generateFile(char *filePath, int numberOfRecords, int recordSize);
int copyFile(char *sourcePath, char *targetPath, int numberOfRecords, int recordSize);
int sortFile(char *filePath, int numberOfRecords, int recordSize);

#endif //ZAD1_LIB_H
