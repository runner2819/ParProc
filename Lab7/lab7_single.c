#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

bool is_prime(int num) {
    if (num == 2) return true;
    if (num < 2 || num % 2 == 0) return false;
    int limit = sqrt(num) + 1;
    bool res = true;
    for (int i = 3; i <= limit; i += 1) {
        if (num % i == 0) {
            res = false;
            break;
        }
    }
    return res;
}

int main(int argc, char **argv) {
    const long n_start = 1;
    const long n_end = 1e7;
    const int tries = 1;

    double start, end, time = 0, ts, te;

    ts = clock();
    long *all_primes;
    int all_primes_cnt = 0;
    for (int _ = 0; _ < tries; _++) {
        all_primes = malloc((n_end - n_start) * sizeof(long));
        start = clock();

        for (int i = n_start; i <= n_end; i++) {
            if (is_prime(i)) {
                all_primes[all_primes_cnt++] = i;
            }
        }

        end = clock();
        time += end - start;
        free(all_primes);
    }
    te = clock();
    printf("total,avg %lf,%lf\n", (te - ts) / CLOCKS_PER_SEC, time / CLOCKS_PER_SEC / tries);
//        for (int i = 0; i < all_primes_cnt; i++) {
//            printf("%ld ", all_primes[i]);
//        }
//        printf("\n");
    return (0);
}