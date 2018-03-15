#ifndef ZAD1_SYS_H
#define ZAD1_SYS_H

#include <stdio.h>
#include <stdlib.h>

int sysGenerateFile(char *filePath, int numberOfRecords, int recordSize);
int sysCopyFile(char *sourcePath, char *targetPath, int numberOfRecords, int recordSize);
int sysSortFile(char *filePath, int numberOfRecords, int recordSize);

#endif //ZAD1_SYS_H
