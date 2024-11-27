#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
//#include <memory.h>
//#include <omp.h>

void generate_array(long *array, const long count, const long seed) {
    srand(seed);
    for (long i = 0; i < count; i++) {
        array[i] = (long) rand() * rand();
    }
}

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number
//    size = 1;
    const long count = 1e8;
    const long random_seed = 920215;
//    const long threads = omp_get_num_procs();
    const int num_seed = 10;

    long *array = malloc(count * sizeof(long));
    long lmax = INT64_MIN;    ///< Local maximums

    long max = INT64_MIN;  ///< The maximal element

    double start, end, time = 0, ts, te;

    ret = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!rank) {
        ts = MPI_Wtime();
    }

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        if (!rank) {
            generate_array(array, count, random_seed + seed_num * 1024);
        }
        MPI_Bcast(array, count, MPI_LONG, 0, MPI_COMM_WORLD);

        const long wstart = (long) (rank) * count / size;
        const long wend = (long) (rank + 1) * count / size;

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();

        for (int i = wstart; i < wend; i++) {
            if (array[i] > lmax) { lmax = array[i]; }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        time += end - start;
//    printf("Processor #%d reports local max = %ld;\n", rank, lmax);

        MPI_Reduce(&lmax, &max, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

//    MPI_Barrier(MPI_COMM_WORLD);
//    double end = MPI_Wtime();
//        if (!rank) {
//            printf("*** Global Maximum for seed %d is %ld;\n", seed_num, max);
//        }

    }
    if (!rank) {
        te = MPI_Wtime();
//        int i = 1;
//        while (i) {
//            sleep(5);
//        }
//        printf("\n*** Global Maximum is %ld;\n", max);
//        printf("Time: %lf\n", end - start);
        printf("total,avg %lf,%lf\n", te - ts, time / num_seed);
//        printf("Eff: %lf\n", 0.070249 / (end - start) / size);
    }
    ret = MPI_Finalize();
//    if (!rank) {
//    main2(count, size, num_seed);
//    }
//    printf("MPI Finalize returned (%d);\n", ret);
    return (0);
}



