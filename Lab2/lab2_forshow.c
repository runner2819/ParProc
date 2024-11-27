#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <math.h>

const int *generate_array(const int count, const int seed) {
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

#pragma omp parallel num_threads(threads) shared(array, count, found, target) reduction(max:index) default(none)
    {
#pragma omp for
        for (int i = 0; i < count; i++) {
            if (found) {
                i = count;
                continue;
            }
            if (array[i] == target) {
                index = i;
#pragma omp atomic write
                found = true;
            }
        }
    }
    double end = omp_get_wtime();
    time_required = end - start;
//    printf("Index is: %d\n", index);
    return time_required;
}

int main(int argc, char **argv) {
    const int count = 5*pow(10, 8);     ///< Number of array elements
    const int random_seed = 920214; ///< RNG seed
    const int target = 16;          ///< Number to look for
//    const int threads = omp_get_num_procs();
    const int threads = 8;
    const int num_seed = 1;

    double *s = calloc(omp_get_num_procs(), sizeof(double));
    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        /* Initialize the RNG */
        int *array = generate_array(count, random_seed + seed_num * 1024);
        for (int n = 1; n <= threads; n++) {
            s[n - 1] += find_index_of_elem(array, count, target, n);
        }
        printf("%d\n", seed_num);
        free(array);
    }
//    printf("single %lf\n", single / num_seed);
    for (int i = 1; i <= threads; i++) {
//        printf("Threads %d - %lf\n", i, s[i - 1] / num_seed);
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
    }
    free(s);

    return 0;
}