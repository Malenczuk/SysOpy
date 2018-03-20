#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "lib.h"
#include "sys.h"

#ifndef ITERATIONS
#define ITERATIONS 1
#endif
typedef struct{
    long int stime;
    long int utime;
    long int rtime;
}Measurement;

static int mode = 0;
static int gen = 0;
static int sort = 0;


long int getTime(struct timeval *t) {
    return (long int)t->tv_sec * 1000000 + (long int)t->tv_usec;
}

void printTimeDiff(Measurement fstMeasurement, Measurement sndMeasurement){
    long int sTime = (sndMeasurement.stime - fstMeasurement.stime) / ITERATIONS;
    long int uTime = (sndMeasurement.utime - fstMeasurement.utime) / ITERATIONS;
    long int rTime = (sndMeasurement.rtime - fstMeasurement.rtime) / ITERATIONS;
    printf("System: %*.*lfs\n",15,6,(double) sTime/1000000);
    printf("User:   %*.*lfs\n",15,6,(double) uTime/1000000);
    printf("Real:   %*.*lfs\n",15,6,(double) rTime/1000000);
}

int main(int argc, char *argv[]) {
    int c;
    char *filePath = NULL;
    char *copyPath = NULL;
    int numberOfRecords = 0, recordSize = 0;
    while (1) {
        static struct option long_options[] =
                {
                        {"lib",    no_argument,       &mode, 1},
                        {"sys",    no_argument,       &mode, 0},
                        {"gen",    no_argument,       &gen,  1},
                        {"sort",   no_argument,       &sort, 1},
                        {"file",   required_argument, 0, 'F'},
                        {"number", required_argument, 0, 'n'},
                        {"size",   required_argument, 0, 's'},
                        {"copy",   required_argument, 0, 'C'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "F:n:s:C:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;
            case 'F':
                printf("option -F with value `%s'\n", optarg);
                filePath = optarg;
                break;
            case 'n':
                printf("option -n with value `%s'\n", optarg);
                numberOfRecords = (int) strtol(optarg, '\0', 10);
                break;
            case 's':
                printf("option -s with value `%s'\n", optarg);
                recordSize = (int) strtol(optarg, '\0', 10);
                break;
            case 'C':
                printf("option -c with value `%s'\n", optarg);
                copyPath = optarg;
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
    }
    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        putchar('\n');
    }

    Measurement measurements[2];
    struct rusage usage;
    struct timeval real;

    getrusage(RUSAGE_SELF, &usage);
    gettimeofday(&real, 0);
    measurements[0].stime = getTime(&usage.ru_stime);
    measurements[0].utime = getTime(&usage.ru_utime);
    measurements[0].rtime = getTime(&real);

    for(int i = 0; i < ITERATIONS; i++) {
        if (mode) {
            if (gen && filePath)
                if (generateFile(filePath, numberOfRecords, recordSize))
                    exit(1);
            if (filePath && copyPath)
                if (copyFile(filePath, copyPath, numberOfRecords, recordSize))
                    exit(1);
            if (sort && filePath)
                if (sortFile(filePath, numberOfRecords, recordSize))
                    exit(1);
        } else {
            if (gen && filePath)
                if (sysGenerateFile(filePath, numberOfRecords, recordSize))
                    exit(1);
            if (filePath && copyPath)
                if (sysCopyFile(filePath, copyPath, numberOfRecords, recordSize))
                    exit(1);
            if (sort && filePath)
                if (sysSortFile(filePath, numberOfRecords, recordSize))
                    exit(1);
        }
    }

    getrusage(RUSAGE_SELF, &usage);
    gettimeofday(&real, 0);
    measurements[1].stime = getTime(&usage.ru_stime);
    measurements[1].utime = getTime(&usage.ru_utime);
    measurements[1].rtime = getTime(&real);

    printTimeDiff(measurements[0], measurements[1]);

    return 0;
}