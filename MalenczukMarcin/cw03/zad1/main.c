#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <ftw.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <zconf.h>
#include <dirent.h>
#include <math.h>
#include <limits.h>

static const char default_format[] = "%d %b %H:%M";
int search = 0;
struct tm *date = NULL;

int intcmp(int a, int b){
    return (a == b ? 0 : a > b ? 1 : -1);
}

int checkDate(struct tm *mtime){
    int sYear  = date->tm_year == -1 ? mtime->tm_year : date->tm_year;
    int sMonth = date->tm_mon  == -1 ? mtime->tm_mon  : date->tm_mon;
    int sDay   = date->tm_mday == -1 ? mtime->tm_mday : date->tm_mday;
    int sHour  = date->tm_hour == -1 ? mtime->tm_hour : date->tm_hour;
    int sMin   = date->tm_min  == -1 ? mtime->tm_min  : date->tm_min;

    return (intcmp(mtime->tm_year,  sYear) != 0 ? intcmp(mtime->tm_year,  sYear) :
            intcmp(mtime->tm_mon,  sMonth) != 0 ? intcmp(mtime->tm_mon,  sMonth) :
            intcmp(mtime->tm_mday,   sDay) != 0 ? intcmp(mtime->tm_mday,   sDay) :
            intcmp(mtime->tm_hour,  sHour) != 0 ? intcmp(mtime->tm_hour,  sHour) :
            intcmp(mtime->tm_min,    sMin) != 0 ? intcmp(mtime->tm_min,    sMin) :
            0) == search;
}

static int displayInfo(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    struct tm mtime;
    char res[32];

    (void) localtime_r(&sb->st_mtime, &mtime);
    if(!checkDate(&mtime)) return 0;
    
    // type
    printf( typeflag == FTW_D  ? "\033[0;34md" : 
            typeflag == FTW_F  ? "\033[0m."    : 
            typeflag == FTW_SL ? "\033[0;36ml" : 
                                 "\033[0m?");
    
    // rights
    int stMode = sb->st_mode;
    printf(stMode & S_IRUSR ? "\033[1;33mr" : "\033[0m-");
    printf(stMode & S_IWUSR ? "\033[1;31mw" : "\033[0m-");
    printf(stMode & S_IXUSR ? "\033[1;32mx" : "\033[0m-");
    printf(stMode & S_IRGRP ? "\033[0;33mr" : "\033[0m-");
    printf(stMode & S_IWGRP ? "\033[0;31mw" : "\033[0m-");
    printf(stMode & S_IXGRP ? "\033[0;32mx" : "\033[0m-");
    printf(stMode & S_IROTH ? "\033[0;33mr" : "\033[0m-");
    printf(stMode & S_IWOTH ? "\033[0;31mw" : "\033[0m-");
    printf(stMode & S_IXOTH ? "\033[0;32mx" : "\033[0m-");
    printf("\033[0m ");
    
    double size = 0;
    // size
    if(typeflag == FTW_D)
        printf("\033[1;32m%*s\033[0m ", 6, "-");
    else{
        size = (double) sb->st_size;
        if(size < 1000)
            printf("\033[1;32m%*.*f\033[0m ", 6, 0, size);
        else {
            int i = 0;
            char *unit = "kMGTPEZY";
            while ((size /= 1000) >= 1000) i++;
            printf("\033[1;32m%*.*f%c\033[0m ", 5, 1, size, unit[i]);
        }
    }

    // owner
    struct passwd *ptmp = getpwuid(sb->st_uid);
    printf("\033[1;33m%s\033[0m ", ptmp->pw_name);  
    
    // date
    strftime(res, sizeof(res), default_format, &mtime);
    printf("\033[0;34m%s\033[0m  ", res);

    // path
    printf("\033[0m%s\n", fpath);

    return 0;  
}


int customExa(const char *dirpath,
              int (*fn)(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)){
    char pathBuff[PATH_MAX+1];
    if(strlen(dirpath) > PATH_MAX) {
        printf("Path length exceeded.\n");
        return 1;
    }
    DIR *dir = opendir(dirpath);
    if (dir == NULL) {
        printf("Error while opening dir at path %s.\n", dirpath);
        return -1;
    }

    struct dirent *dirEntry;
    struct stat st;

    while ((dirEntry = readdir(dir)) != NULL) {
        strcpy(pathBuff, dirpath);
        if(strlen(dirpath) + strlen(dirEntry->d_name) + 1 > PATH_MAX) {
            printf("Path length exceeded.\n");
            return 1;
        }
        strcat(pathBuff, "/");
        strcat(pathBuff, dirEntry->d_name);

        if ((strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)) continue;

        if(lstat(pathBuff, &st) >= 0) {
            if (S_ISDIR(st.st_mode)) {
                fn(pathBuff, &st, FTW_D, NULL);
                pid_t pid = fork();
                if(pid == 0){
                    // printf("Fork\n");
                    customExa(pathBuff, fn);
                    exit(0);
                }
            } else if(S_ISREG(st.st_mode)) {
                fn(pathBuff, &st, FTW_F, NULL);
            } else if(S_ISLNK(st.st_mode)){
                fn(pathBuff, &st, FTW_SL, NULL);
            }
        }
    }

    closedir(dir);
    return 0;
}

void initDate(){
    date = calloc(1, sizeof(struct tm));
    date->tm_year = -1;
    date->tm_mon = -1;
    date->tm_mday = -1;
    date->tm_hour = -1;
    date->tm_min = -1;
}

int main (int argc, char **argv)
{
    int c;
    char *path = ".";
    initDate();
    while (1)
    {
        static struct option long_options[] =
                {
                        {"path",   required_argument, 0, 'p'},
                        {"search", required_argument, 0, 's'},
                        {"year",   required_argument, 0, 'Y'},
                        {"month",  required_argument, 0, 'M'},
                        {"day",    required_argument, 0, 'D'},
                        {"hour",   required_argument, 0, 'h'},
                        {"min",    required_argument, 0, 'm'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:s:Y:M:D:h:m:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'p':
                printf ("option -p with value `%s'\n", optarg);
                path = optarg;
                break;
            case 's':
                printf ("option -s with value `%s'\n", optarg);
                search = strcmp("<", optarg) == 0 ? -1 :
                         strcmp("=", optarg) == 0 ? 0 :
                         strcmp(">", optarg) == 0 ? 1 : 0;
                break;
            case 'Y':
                printf ("option -Y with value `%s'\n", optarg);
                date->tm_year = (int) strtol(optarg, '\0', 10) - 1900;
                break;
            case 'M':
                printf ("option -M with value `%s'\n", optarg);
                date->tm_mon = (int) strtol(optarg, '\0', 10) - 1;
                break;
            case 'D':
                printf ("option -D with value `%s'\n", optarg);
                date->tm_mday = (int) strtol(optarg, '\0', 10);
                break;
            case 'h':
                printf ("option -h with value `%s'\n", optarg);
                date->tm_hour = (int) strtol(optarg, '\0', 10);
                break;
            case 'm':
                printf ("option -m with value `%s'\n", optarg);
                date->tm_min = (int) strtol(optarg, '\0', 10);
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort ();
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }
    char pathBuff[PATH_MAX+1];
    printf("Permissions  Size User  Date Modified Path\n");
//    nftw(realpath(path, pathBuff), displayInfo, 10, FTW_PHYS);
//    printf("\n\n");
    customExa(realpath(path, pathBuff), displayInfo);
    free(date);
    exit (0);
}