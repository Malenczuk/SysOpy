#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <zconf.h>
#include <sys/times.h>

#define FAILURE_EXIT(code, format, ...) { printf(format, ##__VA_ARGS__); exit(code);}
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(c,d) \
   ({ __typeof__ (c) _c = (c); \
       __typeof__ (d) _d = (d); \
     _c > _d ? _d : _c; })

unsigned char **I; //picture
float **K; //filter
unsigned char **J; //picture with applied filter

int H, W, C, M;
int threads;

double time_diff(clock_t start, clock_t end){
    return (double)(end -  start) / sysconf(_SC_CLK_TCK);
}

void fprintf_time(FILE *file, clock_t rStartTime, struct tms tmsStartTime, clock_t rEndTime, struct tms tmsEndTime){
    fprintf(file, "Real:   %.2lf s   ", time_diff(rStartTime, rEndTime));
    fprintf(file, "User:   %.2lf s   ", time_diff(tmsStartTime.tms_utime, tmsEndTime.tms_utime));
    fprintf(file, "System: %.2lf s\n", time_diff(tmsStartTime.tms_stime, tmsEndTime.tms_stime));
}

void* routine(void *args) {
    int c2 = (int) floor(C/2);
    int thread_number = *(int *) args;
    double s;
    int start_y = H * thread_number / threads;
    int end_y = H * (thread_number + 1) / threads;
    for (int y = start_y; y < end_y; ++y) {
        for (int x = 0; x < W; ++x) {
            s = 0;

            for (int h = 0; h < C; ++h)
                for (int w = 0; w < C; ++w) {
                    int i_y = min((H - 1), max(0, y - c2 + h));
                    int i_x = min((W - 1), max(0, x - c2 + w));
                    s += I[i_y][i_x] * K[h][w];
                }
            J[y][x] = (unsigned char) round(s);
        }
    }

    return NULL;
}

void load_picture(char *file_path) {
    FILE *file;
    if ((file = fopen(file_path, "r")) == NULL) FAILURE_EXIT(2, "Opening input file failed");
    fscanf(file, "P2\n");
    fscanf(file, "%d", &W);
    fscanf(file, "%d", &H);
    fscanf(file, "%d", &M);
    I = malloc(H * sizeof(unsigned char*));
    for(int i = 0; i < H; ++i)
        I[i] = malloc(W * sizeof(unsigned char));

    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            fscanf(file, "%d", &I[y][x]);
    fclose(file);
}

void load_filter(char *file_path) {
    FILE *file;
    if ((file = fopen(file_path, "r")) == NULL) FAILURE_EXIT(2, "Opening filter file failed");
    fscanf(file, "%d", &C);
    K = malloc(C * sizeof(float*));
    for(int i = 0; i < C; ++i)
        K[i] = malloc(C * sizeof(float));

    for (int y = 0; y < C; ++y)
        for (int x = 0; x < C; ++x)
            fscanf(file, "%f", &K[y][x]);

    fclose(file);
}

void save_result(char *file_path) {
    FILE *file;
    if ((file = fopen(file_path, "w")) == NULL) FAILURE_EXIT(2, "Opening output file failed");
    fprintf(file, "P2\n%d %d\n%d\n", W, H, M);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x)
            fprintf(file, "%d ", J[y][x]);
        fprintf(file, "\n");
    }
    fclose(file);
}

void save_times(clock_t r_time[2], struct tms tms_time[2]){
    FILE *file;
    if ((file = fopen("Times.txt", "a")) == NULL) FAILURE_EXIT(2, "Opening times file failed");
    fprintf(file, "Number of Threads: %d\n", threads);
    fprintf(file, "Picture size: %dx%d\n", W, H);
    fprintf(file, "Filter size: %dx%d\n", C,C);
    fprintf_time(file, r_time[0], tms_time[0], r_time[1], tms_time[1]);
    fprintf(file, "\n\n");
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 5) FAILURE_EXIT(1, "./filter <thread_number> <picture_file_path> <filter_file_path> <result_file_path>");
    if ((threads = (int) strtol(argv[1], NULL, 10)) <= 0) FAILURE_EXIT(2, "thread_number must be grater than 0");
    char *picture_file_path = argv[2];
    char *filter_file_path = argv[3];
    char *result_file_path = argv[4];
    clock_t r_time[2] = {0, 0};
    struct tms tms_time[2];

    load_picture(picture_file_path);
    load_filter(filter_file_path);

    J = malloc(H * sizeof(unsigned char*));
    for(int i = 0; i < H; ++i)
        J[i] = malloc(W * sizeof(unsigned char));

    pthread_t* thread = malloc(threads * sizeof(pthread_t));

    r_time[0] = times(&tms_time[0]);

    for (int i = 0; i < threads; ++i) {
        int* arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&thread[i], NULL, routine, arg);
    }
    for (int i = 0; i < threads; ++i) {
        void *x;
        pthread_join(thread[i], &x);
    }

    r_time[1] = times(&tms_time[1]);

    save_result(result_file_path);
    save_times(r_time, tms_time);

    return 0;
}
