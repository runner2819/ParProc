#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

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
    const long count = 1e8;
    const long random_seed = 920215;
    const int num_seed = 100;

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
            if (array[i] > lmax) {
                lmax = array[i];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        time += end - start;

        MPI_Reduce(&lmax, &max, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

    }
    if (!rank) {
        te = MPI_Wtime();
        printf("total,avg %lf,%lf\n", te - ts, time / num_seed);
    }
    ret = MPI_Finalize();
    return (0);
}