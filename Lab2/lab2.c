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

double find_index_of_elem(const int *array, const int count, const int target, const int threads) {
    int index = -1;
    double time_required = 0;
    bool found = false;
    double start = omp_get_wtime();
//    int *counters = calloc(threads, sizeof(int));
#pragma omp parallel num_threads(threads) shared(array, count, found, target/*,counters*/) reduction(max:index)default(none)
    {
#pragma omp for
        for (int i = 0; i < count; i++) {
//            counters[omp_get_thread_num()]++;
            if (found) {
                i = count;
                continue;
            }
            if (array[i] == target) {
                index = i;
#pragma atomic
                found = true;
            }
        }
    }
    //    for (int i = 0; i < count; i++) {
//        if (array[i] == target) {
//            index = i;
//            break;
//        }
//    }
//    for (int i = 0; i < threads; i++) {
//        printf("Thread %d - %d\n", i, counters[i]);
//    }
    double end = omp_get_wtime();
    time_required = end - start;
    if (found) printf("Found at %d\n", index);
//    printf("Index is: %d\n", index);
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
//        array[46572108] = 16;
//        array[51027164] = 16;
        for (int n = 1; n <= threads; n++) {
            s[n - 1] += find_index_of_elem(array, count, target, n);
        }
        free(array);
    }
    double test_end = omp_get_wtime();
    printf("Count: %d\nNum_seed: %d\n",count,num_seed);
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