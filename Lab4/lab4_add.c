#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

int count_factors(const long long n) {
    if (n < 2) return 0;
    long long limit = (n + 1) / 2;
    long long count = 0;
    for (int i = 1; i <= limit; i++) {
        if (n % i == 0) {
            count++;
        }
    }
    return count;
}

double time_count_factors(const long long start, const long long end, const long long threads) {
    long long *num_of_factors = malloc((end - start + 1) * sizeof(long long));
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(start, end, num_of_factors) default(none)
    {
#pragma omp for schedule(dynamic, 1)
        for (long long i = start; i <= end; i++) {
            num_of_factors[i - start] = count_factors(i);
        }
    }
    double end_time = omp_get_wtime();
    free(num_of_factors);
    return end_time - start_time;
}

int main() {
    double test_start = omp_get_wtime();
    const long long threads = omp_get_num_procs();  /// to avoid E-cores on my machine
    const int tries = 10;

    long long start = 1, end = 1e6;
    double *s = calloc(omp_get_num_procs(), sizeof(double));

    for (int i = 0; i < tries; i++) {
        printf("%d\n", i);
        for (int n = 1; n <= threads; n++) {
            s[n - 1] += time_count_factors(start, end, n);
        }
    }

    double test_end = omp_get_wtime();

    printf("Total time:%lf\n", test_end - test_start);
    printf("Time:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / tries);
    }
    printf("\nAcceleration:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1]);
    }
    printf("\nEfficiency:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
    }
    printf("\n%lf", s[0] / s[threads - 1] / threads);
    free(s);

    return 0;
}