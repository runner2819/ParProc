#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>

void generate_array(long *array, const int count, const int seed) {
    srand(seed);
    for (int i = 0; i < count; i++) {
        array[i] = rand();
    }
}

double sort(long *array, long len, const int *gaps, const int s_gap) {
    int gap;
    for (int gap_ind = 0; gap_ind < s_gap; gap_ind++) {
        gap = gaps[gap_ind];
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
    return 0;
}

long *merge(long *arr1, long n1, long *arr2, long n2, long *res) {
    long i = 0, j = 0, k = 0;
    while (i < n1 && j < n2)
        if (arr1[i] < arr2[j]) {
            res[k++] = arr1[i++];
        } else {
            res[k++] = arr2[j++];
        }
    if (i < n1) {
        memcpy(res + k, arr1 + i, (n1 - i - 1) * sizeof(long));
    }
    k += n1 - i - 1;
    if (j < n2) {
        memcpy(res + k, arr2 + j, (n2 - j - 1) * sizeof(long));
    }
//    return res;
}

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number
    int magick = 840;
    const int count = 1e7;
    const long random_seed = 920215;
    const int num_seed = 30;

    ret = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

//    if (!rank) {
//        int i = 1;
//        while (i) {
//            sleep(1);
//        }
//    }

    int part_size = (count + size - 1) / size;

    long *array;
    int *counts_m = malloc(magick * sizeof(int));
    int *displs_m = malloc(magick * sizeof(int));
    int offset;
    int part = magick / size;
    for (int i = 0; i < magick; i++) {
        counts_m[i] = (long) (i + 1) * count / magick - (long) i * count / magick;
        if (i % part == 0) {
            offset = 0;
        }
        displs_m[i] = offset;
        offset += counts_m[i];
    }


    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    offset = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < part; j++) {
            counts[i] += counts_m[i * part + j];
        }
        displs[i] = offset;
        offset += counts[i];
    }

/*
    int **gaps = malloc(part * sizeof(int *));
    int *s_gaps = malloc(part * sizeof(int));
    for (int pp = 0; pp < part; pp++) {
        gaps[pp] = malloc(((int) log2(counts[pp]) + 10) * sizeof(int));
        int s_gap = 0;
        for (int step = counts_m[rank * part + pp] / 2; step > 0; step /= 2) {
            gaps[pp][s_gap] = step;
            s_gap++;
        }
        gaps[pp] = realloc(gaps[pp], s_gap * sizeof(int));
        s_gaps[pp] = s_gap;
    }
*/

    int *gaps = malloc(((int) log2(counts_m[rank * part]) + 10) * sizeof(int));
    int s_gap = 0;
    for (int step = counts_m[rank] / 2; step > 0; step /= 2) {
        gaps[s_gap] = step;
        s_gap++;
    }
    gaps = realloc(gaps, s_gap * sizeof(int));

    double start, end, time = 0, ts, te;

    if (!rank) {
        ts = MPI_Wtime();
        array = malloc(count * sizeof(long));
    }


    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        if (!rank) {
            generate_array(array, count, random_seed + seed_num * 1024);
        }
//        int chunk_size = counts_m[rank];
        long *chunk = malloc(part_size * sizeof(long));
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();

        MPI_Scatterv(array, counts, displs, MPI_LONG, chunk, part_size, MPI_LONG, 0, MPI_COMM_WORLD);

        for (int i = 0; i < part; i++) {
//            sort(chunk + displs_m[rank * part + i], counts_m[rank * part + i], gaps[i], s_gaps[i]);
            sort(chunk + displs_m[rank * part + i], counts_m[rank * part + i], gaps, s_gap);
        }

        long ress_size = counts_m[rank * part] + counts_m[rank * part + 1];
        long *ress = malloc(ress_size * sizeof(long));
        merge(chunk, counts_m[rank * part], chunk + displs_m[rank * part + 1], counts_m[rank * part + 1], ress);
        long *tmp;

        for (int i = 2; i < part; i++) {
            int n1 = ress_size, n2 = counts_m[rank * part + i];
            tmp = malloc((n1 + n2) * sizeof(long));
            merge(ress, ress_size, chunk + displs_m[rank * part + i], counts_m[rank * part + i], tmp);
            ress_size += counts_m[rank * part + i];
            free(ress);
            ress = tmp;
        }
        free(chunk);
        chunk = ress;
        int chunk_size = ress_size;

        long *other;
        int step = 1;
        while (step < size) {
            if (rank % (2 * step) == 0) {
                int other_rank = rank + step;
                int other_size;
                if (other_rank < size) {
                    MPI_Recv(&other_size, 1, MPI_INT, other_rank, 0, MPI_COMM_WORLD, &status);
                    other = malloc(other_size * sizeof(long));
                    MPI_Recv(other, other_size, MPI_LONG, other_rank, 0, MPI_COMM_WORLD, &status);
                    long *res = malloc((chunk_size + other_size) * sizeof(long));
                    merge(chunk, chunk_size, other, other_size, res);

                    free(other);
                    free(chunk);

                    chunk = res;
                    chunk_size += other_size;
                }
            } else {
                int near = rank - step;
                MPI_Send(&chunk_size, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
                MPI_Send(chunk, chunk_size, MPI_LONG, near, 0, MPI_COMM_WORLD);
                break;
            }
            step = step * 2;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        time += end - start;
        free(chunk);
    }

    free(counts_m);
    free(displs_m);
    free(counts);
    free(displs);
    free(gaps);
    if (!rank) {
        te = MPI_Wtime();
        free(array);
        printf("total,avg %lf,%lf\n", te - ts, time / num_seed);
    }
    ret = MPI_Finalize();
    return (0);
}