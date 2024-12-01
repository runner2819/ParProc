#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <math.h>

int *generate_array(const int count, const int seed) {
    srand(seed);
    int *array = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        array[i] = rand();
    }
    return array;
}

double find_index_range_of_elem(const int *array, const int count, const int target, const int threads) {
    int l_index = count;
    int r_index = -1;
    double time_required = 0;
    double start = omp_get_wtime();

#pragma omp parallel num_threads(threads) shared(array, count, target/*,counters*/) reduction(max:r_index) reduction(min:l_index) default(none)
    {
#pragma omp for
        for (int i = 0; i < count; i++) {
            if (array[i] == target) {
                if (i > r_index) r_index = i;
                if (i < l_index) l_index = i;
            }
        }
    }
    double end = omp_get_wtime();
    time_required = end - start;
    if (r_index != -1) printf("Found between %d and %d\n", l_index, r_index);
    else printf("Nope\n");
    return time_required;
}

int main(int argc, char **argv) {
    double test_start = omp_get_wtime();
    const int count = pow(10, 9);     ///< Number of array elements
    const int random_seed = 920214; ///< RNG seed
    const int target = 16;          ///< Number to look for
//    const int threads = omp_get_num_procs();
    const int threads = 8;  /// to avoid E-cores on my machine
    const int num_seed = 50;

    double *s = calloc(omp_get_num_procs(), sizeof(double));
    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        printf("-----------------\n");
        printf("Seed %d\n", seed_num);
        /* Initialize the RNG */
        double st = omp_get_wtime();
        int *array = generate_array(count, random_seed + seed_num * 1024);
        printf("time to generate %lf\n", omp_get_wtime() - st);
        for (int n = 1; n <= threads; n++) {
            s[n - 1] += find_index_range_of_elem(array, count, target, n);
        }
        free(array);
    }
    double test_end = omp_get_wtime();
    printf("Count: %d\nNum_seed: %d\n", count, num_seed);
    printf("Total time:%lf\n", test_end - test_start);
    printf("Time:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
    }
    printf("\nTime:\n");
    for (int i = 1; i <= threads; i++) {
        printf("%lf", s[i - 1] / num_seed);
    }
    printf("\nAcceleration:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1]);
    }
    printf("\nEfficiency:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
    }
    free(s);

    return 0;
}