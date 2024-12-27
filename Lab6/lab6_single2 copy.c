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

long *merge(long *arr1, long n1, long *arr2, long n2, int fre) {
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
    if (fre) {
        free(arr1);
        free(arr2);
    }
    return result;
}


int main(int argc, char **argv) {
    int size = 840;    ///< Total number of processors
//    int rank = -1;    ///< This processor's number
    const int count = 1e7;
    const long random_seed = 920215;
    const int num_seed = 1;

    long *array;
    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int offset = 0;
    for (int i = 0; i < size; i++) {
        counts[i] = (long) (i + 1) * count / size - (long) i * count / size;
        displs[i] = offset;
        offset += counts[i];
    }
    int *gaps = malloc(((int) log2(counts[0]) + 10) * sizeof(int));
    int s_gap = 0;
    for (int step = counts[0] / 2; step > 0; step /= 2) {
        gaps[s_gap] = step;
        s_gap++;
    }
    gaps = realloc(gaps, s_gap * sizeof(int));

    double start, end, time = 0, ts, te;

    ts = omp_get_wtime();
    array = malloc(count * sizeof(long));

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        generate_array(array, count, random_seed + seed_num * 1024);
        long *chunk = malloc(count * sizeof(long));
        long **chunks = malloc(size * sizeof(long *));
        start = omp_get_wtime();
        memcpy(chunk, array, count * sizeof(long));
        for (int i = 0; i < size; i++) {
            chunks[i] = chunk + displs[i];
            sort(chunks[i], counts[i], gaps, s_gap);
        }

//        long ress_size = counts[0] + counts[1];
//        long *ress = malloc(counts[0] * sizeof(long));
//        memcpy(ress, chunk, counts[0] * sizeof(long));
//
//        for (int i = 1; i < size; i++) {
//            ress = merge(ress, ress_size, chunks[i], counts[i]);
//            ress_size += counts[i];
//        }
//        free(chunk);
//        free(chunks);
//        free(ress);

        int *chunk_sizes = malloc(size * sizeof(int));
        memcpy(chunk_sizes, counts, size * sizeof(int));
        int ssss = 0;
        for (int step = 1; step < size; step *= 2) {
            for (int fake_rank = 0; fake_rank < size; fake_rank += 2 * step) {
                int chunk_size = chunk_sizes[fake_rank];
                if (fake_rank % (2 * step) == 0) {
                    int other_rank = fake_rank + step;
                    if (other_rank < size) {
                        ssss++;
                        int other_size = chunk_sizes[other_rank];
                        chunks[fake_rank] = merge(chunks[fake_rank], chunk_size, chunks[other_rank], other_size, step != 1);
//                        free(chunks[rank]);

                        chunk_sizes[fake_rank] += other_size;
                    }
                }
            }
        }
        free(chunk);
        free(chunks[0]);
        free(chunks);
        free(chunk_sizes);
//        printf("%d\n", ssss);
//        free(chunk);
//        free(ress);
//        for (int ii = 0; ii < count; ii++) {
//            if (ress[ii] != chunks[0][ii]) {
//                printf("fuck at %d\n", ii);
//            }
//        }
        end = omp_get_wtime();
        time += end - start;
    }

    free(counts);
    free(displs);
//    if (!rank) {
    te = omp_get_wtime();
    free(array);
    printf("total,avg %lf,%lf\n", te - ts, time / num_seed);
    free(gaps);

//    ret = MPI_Finalize();
    return 0;
}