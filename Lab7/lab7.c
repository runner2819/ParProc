#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <math.h>

bool is_prime(int num) {
    if (num == 2) return true;
    if (num < 2 || num % 2 == 0) return false;
    int limit = sqrt(num) + 1;
    bool res = true;
    for (int i = 3; i < limit; i += 1) {
        if (num % i == 0) {
            res = false;
            break;
        }
    }

    return res;
}

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number
//    const long n_start = 1;
//    const long n_end = 1e8;
    long n_end;
    const long count = 1e8;
    const int tries = 1;
    ret = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double start, end, end2, time = 0, time2 = 0, ts, te;

    for (long n_start = 1e8; n_start <= 1e8+100; n_start += 1e7) {
        n_end = n_start + count - 1;
        MPI_Barrier(MPI_COMM_WORLD);

        time = 0;
        time2 = 0;

        if (!rank) {
            ts = MPI_Wtime();
        }
        const long wstart = (long) (rank) * count / size + n_start;
        const long wend = (long) (rank + 1) * count / size + n_start;
        long *local_primes;
        long *all_primes;
        int all_primes_cnt = 0;
        int primes_cnt = 0;
        for (int _ = 0; _ < tries; _++) {
//        if (!rank)printf("%d\n", _);
            local_primes = malloc((wend - wstart) * sizeof(long));

            MPI_Barrier(MPI_COMM_WORLD);
            start = MPI_Wtime();
#pragma omp parallel num_threads(2) shared(local_primes, wstart, wend, primes_cnt, rank) default(none)
            {
//            double tt = omp_get_wtime();
#pragma omp for
                for (int i = wstart; i < wend; i++) {
                    if (is_prime(i)) {
#pragma omp critical
                        local_primes[primes_cnt++] = i;
                    }
                }
//            tt = omp_get_wtime() - tt;
//            printf("%d %d %lf\n", rank, omp_get_thread_num(), tt);
            }
            end = MPI_Wtime();
            int *recvcounts, *displs;
            if (!rank) {
                recvcounts = malloc(size * sizeof(int));
                displs = malloc(size * sizeof(int));
            }

            MPI_Gather(&primes_cnt, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if (!rank) {
                for (int i = 0; i < size; ++i) {
                    displs[i] = all_primes_cnt;
                    all_primes_cnt += recvcounts[i];
                }

                all_primes = malloc(all_primes_cnt * sizeof(long));
            }

            MPI_Gatherv(local_primes, primes_cnt, MPI_LONG, all_primes, recvcounts, displs, MPI_LONG, 0, MPI_COMM_WORLD);

            time += end - start;
            MPI_Barrier(MPI_COMM_WORLD);
            end2 = MPI_Wtime();
            time2 += end2 - start;
            free(local_primes);
            if (!rank) {
                free(all_primes);
                free(recvcounts);
                free(displs);
            }
        }

        if (!rank) {
            te = MPI_Wtime();
            printf("total, avg %lf %lf\n", te - ts, time2 / tries);
            printf("%d-%d\n", n_start, n_end);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Process %d computing %lf\n", rank, time / tries);
    }

    ret = MPI_Finalize();
    return (0);
}