#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <omp.h>
//#include <math.h>

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number

    const long long count = 1e8;
    const long long random_seed = 920215;
    const long long threads = omp_get_num_procs();
    const int num_seed = 100;

    long long *array = malloc(count * sizeof(long long));
    int lmax = -1;    ///< Local maximums
    int max = -1;  ///< The maximal element

    double start, end, time, ts, te;

    /* Initialize the MPI */
    ret = MPI_Init(&argc, &argv);

    /* Determine our rankand processor count */
    MPI_Comm_size(MPI_COMM_WORLD, &size);
//    printf("MPI Comm Size: %d;\n", size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    printf("MPI Comm Rank: %d;\n", rank);
    if (!rank) {
//        printf("MPI Init returned (%d);\n", ret);
        ts = MPI_Wtime();
    }

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {

//    int i = 0;
//    while (!i)
//        sleep(5);
        /* Master generates the array */
        if (!rank) {
            /* Initialize the RNG */
            srand(random_seed + seed_num * 1024);
            /* Generate the random array */
            for (int i = 0; i < count; i++) {
                array[i] = (int) rand();
            }
        }
//        MPI_Barrier(MPI_COMM_WORLD);
//        printf("Processor #%d has array: ", rank);
//        for (int i = 0; i < count; i++) { printf("%d ", array[i]); }
//        printf("\n");
//    MPI_Barrier(MPI_COMM_WORLD);
//    double start = MPI_Wtime();
        /* Send the array to all other processors */
        MPI_Bcast(array, count, MPI_INTEGER, 0, MPI_COMM_WORLD);

//        printf("Processor #%d has array: ", rank);
//        for (int i = 0; i < count; i++) { printf("%d ", array[i]); }
//        printf("\n");

        const long wstart = (long) (rank) * count / size;
        const long wend = (long) (rank + 1) * count / size;
//    printf("Processor #%d checks items %ld .. %ld;\n", rank, wstart, wend - 1);

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();

        for (int i = wstart; i < wend; i++) {
            if (array[i] > lmax) { lmax = array[i]; }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        time += end - start;
//    printf("Processor #%d reports local max = %ld;\n", rank, lmax);

        MPI_Reduce(&lmax, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

//    MPI_Barrier(MPI_COMM_WORLD);
//    double end = MPI_Wtime();
        if (!rank) {
            printf("\n*** Global Maximum for seed %d is %ld;\n", seed_num, max);
        }

    }
    if (!rank) {
        te = MPI_Wtime();
//        printf("\n*** Global Maximum is %ld;\n", max);
//        printf("Time: %lf\n", end - start);
        printf("total,avg %lf,%lf", te - ts, time / num_seed);
//        printf("Eff: %lf\n", 0.070249 / (end - start) / size);
    }
    ret = MPI_Finalize();
//    printf("MPI Finalize returned (%d);\n", ret);
    return (0);
}


void generate_array(long long *array, const long long count, const long long seed) {
    srand(seed);
    for (long long i = 0; i < count; i++) {
        array[i] = (long long) rand() * rand();
    }
}

double time_find_max_static(const long long *array, const long long count, const long long threads) {
    long long max = INT64_MIN;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(static)
        for (long long i = 0; i < count; i++) {
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

int main2() {
    double test_start = omp_get_wtime();
    const long long count = 1e8;
    const long long random_seed = 920215;
    const long long threads = omp_get_num_procs();
    const int num_seed = 100;

    long long *array = malloc(count * sizeof(long long));
    double *s = calloc(omp_get_num_procs(), sizeof(double));
//    double *s5 = calloc(omp_get_num_procs(), sizeof(double));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        generate_array(array, count, random_seed + seed_num * 1024);

        for (int n = 1; n <= threads; n++) {
            s[n - 1] += time_find_max_static(array, count, n);
//            s1[n - 1] += time_find_max_static(array, count, n);
//            s2[n - 1] += time_find_max_dynamic4(array, count, n);
//            s3[n - 1] += time_find_max_guided1(array, count, n);
//            s4[n - 1] += time_find_max_guided4(array, count, n);
//            s5[n - 1] += time_find_max_auto(array, count, n);
        }
    }
    double test_end = omp_get_wtime();
    free(array);

    printf("\nCount: %lld\nNum_seed: %d\n", count, num_seed);
    printf("Total time:%lf\n", test_end - test_start);
    print_stats(s, threads, num_seed);

    free(s);
    return 0;
}