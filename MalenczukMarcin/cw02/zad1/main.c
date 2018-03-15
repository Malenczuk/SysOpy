#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "lib.h"
#include "sys.h"

static int mode;

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
                        {"file",   required_argument, 0, 'F'},
                        {"number", required_argument, 0, 'n'},
                        {"size",   required_argument, 0, 's'},
                        {"copy",   required_argument, 0, 'c'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "F:n:s:c:",
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
            case 'c':
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
    if(mode) {
        if (filePath)
            if (generateFile(filePath, numberOfRecords, recordSize))
                printf("1");
        if (filePath && copyPath)
            if (copyFile(filePath, copyPath, numberOfRecords, recordSize))
                printf("2");
        if (filePath && copyPath)
            if (sortFile(copyPath, numberOfRecords, recordSize))
                printf("3");
    } else {
        if(filePath)
            if(sysGenerateFile(filePath, numberOfRecords, recordSize))
                printf("1");
        if(filePath && copyPath)
            if(sysCopyFile(filePath, copyPath, numberOfRecords, recordSize))
                printf("2");
        if(filePath && copyPath)
            if(sysSortFile(copyPath, numberOfRecords, recordSize))
                printf("3");
    }
    return 0;
}