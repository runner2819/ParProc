#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

void generate_array(long *array, const long count, const long seed) {
    srand(seed);
    for (long i = 0; i < count; i++) {
        array[i] = (long) rand() * rand();
    }
}

double time_find_max_static(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(static)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_static3(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(static, 1000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_static4(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(static, 10000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_dynamic3(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic, 1000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_dynamic1(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_dynamic4(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(dynamic, 10000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_guided2(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided, 100)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_guided3(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided, 1000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_guided4(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided, 10000)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_guided1(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(guided)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

double time_find_max_auto(const long *array, const long count, const long threads) {
    long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(auto)
        for (long i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

void print_stats(const double *s, const int threads, const int num_seed) {
    printf("Time:\n\\def \\time{");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
    }
    printf("}");
    printf("\nAcceleration:\n\\def \\acc{");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1]);
    }
    printf("}");
    printf("\nEfficiency:\n\\def \\eff{");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
    }
    printf("}\n");
}

int main() {
    double test_start = omp_get_wtime();
    const long count = pow(10, 8);
    const long random_seed = 920215;
    const long threads = omp_get_num_procs();
    const int num_seed = 10;

    long *array = malloc(count * sizeof(long));
    double *s1 = calloc(omp_get_num_procs(), sizeof(double));
    double *s2 = calloc(omp_get_num_procs(), sizeof(double));
    double *s3 = calloc(omp_get_num_procs(), sizeof(double));
    double *s4 = calloc(omp_get_num_procs(), sizeof(double));
//    double *s5 = calloc(omp_get_num_procs(), sizeof(double));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        generate_array(array, count, random_seed + seed_num * 1024);

        for (int n = 1; n <= threads; n++) {
            s1[n - 1] += time_find_max_static(array, count, n);
//            s1[n - 1] += time_find_max_static3(array, count, n);
//            s2[n - 1] += time_find_max_static4(array, count, n);
//            s3[n - 1] += time_find_max_dynamic3(array, count, n);
//            s4[n - 1] += time_find_max_guided3(array, count, n);
//            s1[n - 1] += time_find_max_static(array, count, n);
//            s2[n - 1] += time_find_max_dynamic4(array, count, n);
//            s3[n - 1] += time_find_max_guided1(array, count, n);
//            s4[n - 1] += time_find_max_guided4(array, count, n);
//            s5[n - 1] += time_find_max_auto(array, count, n);
        }
    }
    double test_end = omp_get_wtime();
    free(array);

    printf("static 1000,static 10000,dynamic 1000,guided 1000\n");
    printf("\nCount: %lld\nNum_seed: %d\n", count, num_seed);
    printf("Total time:%lf\n", test_end - test_start);
    print_stats(s1, threads, num_seed);
//    print_stats(s2, threads, num_seed);
//    print_stats(s3, threads, num_seed);
//    print_stats(s4, threads, num_seed);
//    print_stats(s5, threads, num_seed);

    free(s1);
//    free(s2);
//    free(s3);
//    free(s4);
//    free(s5);
    return 0;
}