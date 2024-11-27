#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

double time_find_max(const int *array, const int threads, const int count) {
    int max = -1;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

int comp_find_max(const int *array, const int threads, const int count) {
    int max = -1;
    int counter = 0;
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) reduction(+: counter) default(none)
    {
#pragma omp for
        for (int i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
            counter++;
        }
    }
    return counter;
}

int main(int argc, char **argv) {
    const int count = pow(10, 8);     ///< Number of array elements
    const int random_seed = 920215; ///< main RNG seed
    const int threads = omp_get_num_procs();
    const int num_seed = 25;
    /* Determine the OpenMP support */
    printf("OpenMP: %d;\n======\n", _OPENMP);

    double *times = calloc(omp_get_num_procs(), sizeof(double));
    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        int *array = 0;                 ///< The array we need to find the max in
        /* Initialize the RNG */
        srand(random_seed + seed_num * 1024);

        /* Generate the random array */
        array = (int *) malloc(count * sizeof(int));
        for (int i = 0; i < count; i++) {
            array[i] = rand();
        }

        for (int n = 1; n <= threads; n++) {
            times[n - 1] += time_find_max(array, n, count);
        }
        printf("%d\n", seed_num);
        free(array);
    }

    for (int i = 1; i <= threads; i++) {
        printf("Threads %d - %lf", i, times[i - 1] / num_seed);
    }
    free(times);
    return 0;
}