#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <omp.h>
#include <math.h>

//void save_array(FILE *f, const long *array, const long count) {
//    fwrite(array, sizeof(long), count, f);
//}
//
//void read_array(FILE *f, long *array, const long count) {
//    fread(array, sizeof(long), count, f);
//}
//
//void generate_array(FILE *f, long *array, const long count, const long seed) {
//    srand(seed);
//    for (long i = 0; i < count; i++) {
//        array[i] = (long) rand() * rand();
//    }
//    save_array(f, array, count);
//}


void generate_array(int *array, const int count, const int seed) {
    srand(seed);
    for (int i = 0; i < count; i++) {
        array[i] = rand();
    }
}

double sort(long *array, long len, const int *gaps, const int s_gap) {
    double time_required;
//    double start = omp_get_wtime();
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
//    double end = omp_get_wtime();
//    time_required = end - start;
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

//void shellsort(int numbers[], int array_size) {
//    int i, j, increment, temp;
//    increment = 3;
//    while (increment > 0) {
//        for (i = 0; i < array_size; i++) {
//            j = i;
//            temp = numbers[i];
//            while ((j >= increment) && (numbers[j - increment] > temp)) {
//                numbers[j] = numbers[j - increment];
//                j = j - increment;
//            }
//            numbers[j] = temp;
//        }
//        if (increment / 2 != 0)
//            increment = increment / 2;
//        else if (increment == 1)
//            increment = 0;
//        else
//            increment = 1;
//    }
//}

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number
    const int count = 1e1;
    const long random_seed = 920215;
    const int num_seed = 1;

    int part_size = (count + size - 1) / size;

    ret = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

    long *array;
    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        counts[i] = (long) (i + 1) * count / size - (long) i * count / size;
        displs[i] = (long) i * count / size;
    }

    int chunk_size = counts[rank];

    long *chunk = malloc(part_size * sizeof(long));

    int *gaps = malloc(((int) log2(chunk_size) + 10) * sizeof(int));
    int s_gap = 0;
    for (int step = chunk_size / 2; step > 0; step /= 2) {
        gaps[s_gap] = step;
        s_gap++;
    }
    gaps = realloc(gaps, s_gap * sizeof(int));

    double start, end, time = 0, ts, te;
//    if (!rank) {
//        int i = 1;
//        while (i) {
//            sleep(5);
//        }
//    }

    if (!rank) {
        ts = MPI_Wtime();
        array = malloc(count * sizeof(long));
    }

    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        if (!rank) {
            for (int i = 0; i < count; i++)
                array[i] = count - i;
//            generate_array(array, count, random_seed + seed_num * 1024);
//            MPI_Scatterv(array, counts,)
        }
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();

        MPI_Scatterv(array, counts, displs, MPI_LONG, chunk, part_size, MPI_LONG, 0, MPI_COMM_WORLD);
        sort(chunk, chunk_size, gaps, s_gap);

        long *other;
        int step = 1;
        while (step < size) {
            if (rank % (2 * step) == 0) {
                int other_rank = rank + step;
                int other_size = counts[other_rank];
                if (other_rank < size) {
                    other = malloc(other_size * sizeof(long));
                    MPI_Recv(other, other_size, MPI_LONG, other_rank, 0, MPI_COMM_WORLD, &status);
                    long *res = merge(chunk, chunk_size, other, other_size);

                    free(other);
                    free(chunk);

                    chunk = res;
                    chunk_size += other_size;
                }
            } else {
                int near = rank - step;
                MPI_Send(chunk, chunk_size, MPI_LONG, near, 0, MPI_COMM_WORLD);
                free(chunk);
                break;
            }
            step = step * 2;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        time += end - start;
//    printf("Processor #%d reports local max = %ld;\n", rank, lmax);

//        MPI_Reduce(&lmax, &max, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

//    MPI_Barrier(MPI_COMM_WORLD);
//    double end = MPI_Wtime();
//        if (!rank) {
//            printf("*** Global Maximum for seed %d is %ld;\n", seed_num, array[1]);
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
        for (int i = 0; i < count; i++)
            printf("%ld\n", array[i]);
//        printf("Eff: %lf\n", 0.070249 / (end - start) / size);
    }
    ret = MPI_Finalize();
//    if (!rank) {
//        fclose(f);
//    main2(count, size, num_seed);
//    }
//    printf("MPI Finalize returned (%d);\n", ret);
    return (0);
}



