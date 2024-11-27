#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "omp.h"
#include <unistd.h>

int main(int argc, char **argv) {
    const int threads = 8;
    for (long count = 1000; count < 5000000000; count *= 3.16227766) {
        double avg_thread_time = 0;
        double avg_linear_time = 0;
        for (int random_seed = 1; random_seed < 11; ++random_seed) {
            srand(920214 + random_seed * 1024);
            int *array = 0;
            int max = -1;
            array = (int *) malloc(count * sizeof(int));
            for (int i = 0; i < count; i++) { array[i] = rand(); }

            double time_start = omp_get_wtime();

#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
            {
#pragma omp for
                for (int i = 0; i < count; i++) {
                    if (array[i] > max) { max = array[i]; };
                }
            }

            double time_finish = omp_get_wtime();
            avg_thread_time += (time_finish - time_start);

            // linear part
            clock_t begin = clock();
            max = -1;
            for (int i = 0; i < count; ++i) {
                if (array[i] > max) { max = array[i]; };
            }
            clock_t end = clock();
            avg_linear_time += (end - begin) / CLOCKS_PER_SEC;
            free(array);

        }
        avg_thread_time /= 10;
        avg_linear_time /= 10;


        printf("%ld : ", count);
        double acc = avg_linear_time / avg_thread_time;
        printf("%lf / %lf = %lf\n", avg_linear_time, avg_thread_time, acc);
    }
    return (0);
}