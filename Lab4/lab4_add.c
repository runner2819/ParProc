#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

void generate_array(long long *array, const long long count, const long long seed) {
    srand(seed);
    for (long long i = 0; i < count; i++) {
//        arc4random_buf(array + i, sizeof(long long));
        array[i] = (long long) rand() * rand();
    }
}


double time_find_max_static(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
//    printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_dynamic3(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic, 1000)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
//    printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_dynamic4(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic, 10000)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    //printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_guided3(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided, 1000)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    //printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_guided4(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided, 10000)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    //printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_guided1(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    //printf("%lld\n", max);
    return end_time - start_time;
}

double time_find_max_auto(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(auto)
        for (long long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    //printf("%lld\n", max);
    return end_time - start_time;
}

double main2(double (*time)(const long long *, const long long, const long long)) {
    double test_start = omp_get_wtime();
    const long long count = pow(10, 3);     ///< Number of array elements
    const long long random_seed = 920215; ///< RNG seed
    const long long threads = omp_get_num_procs();  /// to avoid E-cores on my machine
    const int num_seed = 10;

    long long *array = malloc(count * sizeof(long long));
    double *s = calloc(omp_get_num_procs(), sizeof(double));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
//        printf("-----------------\n");
//        printf("Seed %d\n", seed_num);

//        double st = omp_get_wtime();
        generate_array(array, count, random_seed + seed_num * 1024);
//        printf("time to generate %lf\n", omp_get_wtime() - st);

        for (int n = 1; n <= threads; n++) {
            s[n - 1] += time(array, count, n);
        }
    }
    double test_end = omp_get_wtime();
    free(array);
    printf("\nCount: %lld\nNum_seed: %d\n", count, num_seed);
    printf("Total time:%lf\n", test_end - test_start);
    printf("Time:\n\\def \\time{");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
    }
    printf("}");
//    printf("\nTime:\n");
//    for (int i = 1; i <= threads; i++) {
//        printf("%lf ", s[i - 1] / num_seed);
//    }
    printf("\nAcceleration:\n\\def \\acc");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1]);
    }
    printf("}");
    printf("\nEfficiency:\n\\def \\eff");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
    }
    printf("}\n");
    free(s);
    return test_end - test_start;
}

int main() {
    double s = 0;
    s += main2(time_find_max_static);
//    s += main2(time_find_max_dynamic3);
    s += main2(time_find_max_dynamic4);
    s += main2(time_find_max_guided1);
//    s += main2(time_find_max_guided3);
    s += main2(time_find_max_guided4);
    s += main2(time_find_max_auto);
    printf("Totallest total time:%lf", s);
    return 0;
}