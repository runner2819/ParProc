#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
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

long *merge(long *arr1, long n1, long *arr2, long n2) {
    long *result = malloc((n1 + n2) * sizeof(long));
    long i = 0, j = 0, k = 0;

    while (i < n1 && j < n2)
        if (arr1[i] < arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    while (i < n1) {
        result[k++] = arr1[i++];
    }
    while (j < n2) {
        result[k++] = arr2[j++];
    }

    return result;
}

int main(int argc, char **argv) {
    int size = 840;    ///< Total number of processors
//    int rank = -1;    ///< This processor's number
    const int count = 1e8;
    const long random_seed = 920215;
    const int num_seed = 30;

    long *array;
    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int offset = 0;
    for (int i = 0; i < size; i++) {
        counts[i] = (long) (i + 1) * count / size - (long) i * count / size;
        displs[i] = offset;
        offset += counts[i];
    }
    int **gaps = malloc(size * sizeof(int *));
    int *s_gaps = malloc(size * sizeof(int));
    for (int rank = 0; rank < size; rank++) {
        gaps[rank] = malloc(((int) log2(counts[rank]) + 10) * sizeof(int));
        int s_gap = 0;
        for (int step = counts[rank] / 2; step > 0; step /= 2) {
            gaps[rank][s_gap] = step;
            s_gap++;
        }
        gaps[rank] = realloc(gaps[rank], s_gap * sizeof(int));
        s_gaps[rank] = s_gap;
    }

    double start, end, time = 0, ts, te;

    ts = omp_get_wtime();
    array = malloc(count * sizeof(long));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        generate_array(array, count, random_seed + seed_num * 1024);
        long **chunks = malloc(size * sizeof(long *));
        start = omp_get_wtime();
        for (int rank = 0; rank < size; rank++) {
            int chunk_size = counts[rank];
            chunks[rank] = malloc(chunk_size * sizeof(long));
            memcpy(chunks[rank], array + displs[rank], chunk_size * sizeof(long));
            sort(chunks[rank], chunk_size, gaps[rank], s_gaps[rank]);
        }

        int *chunk_sizes = malloc(size * sizeof(int));
        memcpy(chunk_sizes, counts, size * sizeof(int));
        for (int step = 1; step < size; step *= 2) {
            for (int rank = 0; rank < size; rank += 2 * step) {
                int chunk_size = chunk_sizes[rank];
                if (rank % (2 * step) == 0) {
                    int other_rank = rank + step;
                    if (other_rank < size) {
                        int other_size = chunk_sizes[other_rank];
                        long *res = merge(chunks[rank], chunk_size, chunks[other_rank], other_size);

                        free(chunks[rank]);
                        free(chunks[other_rank]);

                        chunks[rank] = res;
                        chunk_sizes[rank] += other_size;
                    }
                }
            }
        }

        end = omp_get_wtime();
        time += end - start;
        free(chunks[0]);
        free(chunks);
        free(chunk_sizes);
    }

    free(counts);
    free(displs);
    free(gaps);
//    if (!rank) {
    te = omp_get_wtime();
    free(array);
    printf("total,avg %lf,%lf\n", te - ts, time / num_seed);
//    }
//    ret = MPI_Finalize();
    return (0);
}