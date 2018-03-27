#include <stdlib.h>
#include <stdio.h>

int ARR_SIZE = 1073741824;

int main() {
    int *arr = malloc(sizeof(int) * ARR_SIZE);
    for (int i = 0; i< ARR_SIZE; i++) {
        arr[i] = '^';
    }
    perror("Allocation ");
}