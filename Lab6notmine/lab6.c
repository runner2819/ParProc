#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int GetStartKnuthStep(int n)
{
    int step = 1;
    while (step <= n)
        step = step * 3 + 1;
    return step;
}

int GenArr(int* arr, int n, int rng_seed)
{
    srand(rng_seed);
    if (!arr) return -1;
    for (int i = 0; i < n; ++i)
        arr[i] = rand();
    return 0;
}

double ShellSort(int* array, int n, int start_step)
{
    if (!array) return -1;
    double start_t = 0, end_t = 0;
    start_t = MPI_Wtime();

    for (int step = start_step; step > 0; step /= 3)
    {
        printf("step: %d\n", step);
        for (int i = step; i < n; i += step)
        {
            int temp = array[i], j;
            for (j = i; j >= step && array[j - step] > temp; j -= step)
                array[j] = array[j - step];
            array[j] = temp;
        }
    }

    end_t = MPI_Wtime();
    return end_t - start_t;
}

double ShellSortP(int* array, int n)
{
    int n_threads = -1, rank = -1;
    double full_t  = 0, merge_t = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_threads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int chunk_size = n / n_threads;
    if (rank < n % n_threads) chunk_size += 1;

    int step         = GetStartKnuthStep(chunk_size);
    int step_n       = GetStartKnuthStep(n);
    int* local_array = (int*)calloc(chunk_size, sizeof(int));

    printf("%d %d\n", rank, chunk_size);

    // send data to local arrays
    MPI_Scatter(
        array,       chunk_size, MPI_INT,
        local_array, chunk_size, MPI_INT,
        0, MPI_COMM_WORLD
    );

    // timing
    full_t = ShellSort(local_array, chunk_size, step);

    MPI_Gather(
        local_array, chunk_size, MPI_INT,
        array,       chunk_size, MPI_INT,
        0, MPI_COMM_WORLD
    );

    // finalize to sort the array
    if (!rank) merge_t = ShellSort(array, n, step_n);
    MPI_Barrier(MPI_COMM_WORLD);

    return full_t + merge_t;
}

int main(int argc, char* argv[])
{
    // mpirun -np <proc> ./lab6 7 10
    int size_pow = atoi(argv[1]);
    int repeats  = atoi(argv[2]);
    int rng_seed = 515525;
    int rng_step = 1024;
    int size     = pow(10, size_pow);
    int rank     = 0;
    int status   = 0, num_proc   = -1;
    double max_time = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double avg_time = 0;
    for (int seed = rng_seed; seed < rng_seed + repeats * rng_step; seed += rng_step)
    {
        printf("seed: %d\n", seed);
        int* array = (int*)calloc(size, sizeof(int));
        GenArr(array, size, seed);
        avg_time += ShellSortP(array, size);
    }
    MPI_Reduce(&avg_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (!rank) printf("[RESULT]: avg time is: %lf", avg_time / repeats);
    status = MPI_Finalize();
    return 0;
}