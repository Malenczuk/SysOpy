#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define ARGS_MAX 64
#define CMDS_MAX 64
#define LINE_MAX 256
int pipes[2][2];

char *
strtok2(s, delim)
        char *s;            /* string to search for tokens */
        const char *delim;  /* delimiting characters */
{
    static char *lasts;
    register int ch;

    if (s == 0)
        s = lasts;
    do {
        if ((ch = *s++) == '\0')
            return 0;
    } while (strchr(delim, ch));
    --s;
    lasts = s + strcspn(s, delim);
    if (*lasts != 0)
        *lasts++ = 0;
    return s;
}

int main(int argc, char *argv[]) {
    if (argc < 2) FAILURE_EXIT(1, "To little Arguments.\n");
    FILE *file = fopen(argv[1], "r");
    if (!file) FAILURE_EXIT(1, "Error while opening file %s\n", argv[1]);

    char r0[LINE_MAX], r1[LINE_MAX];
    char *args[ARGS_MAX], *cmds[CMDS_MAX];
    int red[CMDS_MAX];
    for (int i = 0; i < CMDS_MAX; i++) red[i] = 0;
    int cmdNum = 0, argNum = 0, lineNum = 0;
    char *savepCMD, *savepRED, *savepARG;

    while (fgets(r0, LINE_MAX, file)) {
        lineNum++;
        strcpy(r1, r0);
        cmdNum = 0;
        while ((cmds[cmdNum] = strtok_r(cmdNum == 0 ? r1 : NULL, "|\n", &savepCMD)) != NULL){
            if((strstr(cmds[cmdNum], ">")) != 0) {
                int append = strstr(cmds[cmdNum], ">") == strstr(cmds[cmdNum], ">>") ? 2 : 1;
                cmds[cmdNum] = strtok_r(cmds[cmdNum], ">", &savepRED);
                cmdNum++;
                while((cmds[cmdNum] = strtok_r(NULL, ">", &savepRED)) != NULL) {
                    red[cmdNum] = append;
                    append = strstr(cmds[cmdNum], ">") == strstr(cmds[cmdNum], ">>") ? 2 : 1;
                    cmdNum++;
                }
                cmdNum--;
            }
            if (++cmdNum >= CMDS_MAX) FAILURE_EXIT(1, "Line %d exceeds maximum number (%d) of pipes", lineNum, CMDS_MAX)
        }
        if (!cmdNum) continue;
        for(int i = 0; i < cmdNum; i++) printf("%d %s, ", red[i] ,cmds[i]);
        printf("\n");
        int k;
        for (k = 0; k < cmdNum; k++) {
            argNum = 0;
            while ((args[argNum] = strtok_r(argNum == 0 ? cmds[k] : NULL, " \t\n>", &savepARG)) != NULL)
                if (++argNum >= ARGS_MAX) FAILURE_EXIT(1, "Command %d exceeds maximum number (%d) of args", k, ARGS_MAX)
            if (!argNum) continue;

            if (k > 1) {
                close(pipes[k % 2][0]);
                close(pipes[k % 2][1]);
            }
            if (pipe(pipes[k % 2]))
                printf("Couldn't pipe at number %d!\n", k);

            pid_t cp = fork();
            if (cp == -1) FAILURE_EXIT(1, "Couldnt fork child process %d!\n", k);
            if (cp == 0) {
                if (k < cmdNum - 1) {
                    close(pipes[k % 2][0]);
                    if (dup2(pipes[k % 2][1], 1) < 0) FAILURE_EXIT(3, "Couldnt set writing at number %d!\n", k);
                    wait(NULL);
                }
                if (k != 0) {
                    close(pipes[(k + 1) % 2][1]);
                    if (dup2(pipes[(k + 1) % 2][0], 0) < 0) FAILURE_EXIT(3, "Couldnt set reading at number %d!\n", k);
                }
                if(red[k] > 0) execlp("./redirect", "./redirect", (red[k] == 1 ? ">" : ">>"), args[0], NULL);
                else execvp(args[0], args);
                FAILURE_EXIT(1, "ERROR EXECUTING CHILD PROCESS %d!\n", k);
            }
        }
        close(pipes[k % 2][0]);
        close(pipes[k % 2][1]);
        while (wait(NULL)) {
            if (errno == ECHILD) {
                printf("\n");
                break;
            }
        }
    }
    fclose(file);
    return 0;
}