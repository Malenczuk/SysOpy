#include <stdlib.h>
#include <stdio.h>

int fib(int n) {
    return n <= 1 ? 0 : fib(n - 1) + fib(n - 2);
}

int main() {
    fib(50);
    perror("Fib");
    return 0;
}
