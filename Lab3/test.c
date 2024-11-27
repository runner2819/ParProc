#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <memory.h>

void generate_array(int *array, const int count, const int seed) {
    srand(seed);
    for (int i = 0; i < count; i++) {
        array[i] = rand();
    }
}

double sort(int *array, int len, const int threads, const int *gaps, const int s_gap) {
    double time_required;
    double start = omp_get_wtime();
    int gap;

    for (int gap_ind = 0; gap_ind < s_gap; gap_ind++) {
        gap = gaps[gap_ind];
#pragma omp parallel for num_threads(threads) shared(array, len, gap, l) default(none)
        for (int offset = 0; offset < gap; offset++) { // iterate through insert sortings
            int temp, j;
            for (int i = gap + offset; i < len; i += gap) {
                temp = array[i];
                for (j = i; (j >= gap) && (array[j - gap] > temp); j -= gap) {
                    array[j] = array[j - gap];
                }
                array[j] = temp;
            }
        }
    }
    double end = omp_get_wtime();
    time_required = end - start;
    return time_required;
}

int main(int argc, char **argv) {
    double test_start = omp_get_wtime();
    const int count = pow(10, 7);     ///< Number of array elements
    const int random_seed = 920214; ///< RNG seed
    const int threads = 8;  /// to avoid E-cores on my machine
    const int num_seed = 10;

    int *gaps = malloc(((int) log2(count) + 10) * sizeof(int));
    int s_gap = 0;
    for (int step = count / 2; step > 0; step /= 2) {
        gaps[s_gap] = step;
        s_gap++;
    }
    gaps = realloc(gaps, s_gap * sizeof(int));

    int *array = malloc(count * sizeof(int));
    int *clone = malloc(count * sizeof(int));
    double *s = calloc(omp_get_num_procs(), sizeof(double));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        printf("-----------------\n");
        printf("Seed %d\n", seed_num);

        double st = omp_get_wtime();
        generate_array(array, count, random_seed + seed_num * 1024);
//        memcpy(clone, array, count * sizeof(int));

        printf("time to generate %lf\n", omp_get_wtime() - st);
        s[0] += sort(array, count, 1, gaps, s_gap);
        memcpy(array, clone, count * sizeof(int));
        s[7] += sort(array, count, 8, gaps, s_gap);
//        for (int n = 1; n <= threads; n++) {
//            s[n - 1] += sort(array, count, n, gaps, s_gap);
//            memcpy(array, clone, count * sizeof(int));
//        }
    }
    double test_end = omp_get_wtime();
    free(array);
    free(clone);
    free(gaps);

    printf("Count: %d\nNum_seed: %d\n", count, num_seed);
    printf("Total time:%lf\n", test_end - test_start);
    printf("Time:\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
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