// ConsoleApplication2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

double seek_once(const int count, const int threads) {

    const int random_seed = (int) (903459); ///< RNG seed

    int *array = 0;                 ///< The array we need to find the max in
    int index = -1;                ///< The index of the element we need
    bool found = false;

    /* Initialize the RNG */
    srand(random_seed);

    /* Generate the random array */
    array = (int *) malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) { array[i] = rand(); }
//    const int target = 16;          ///< Number to look for
    int t = rand() % count;
    const int target = array[rand() % count];          ///< Number to look for

    // Timing
    double begin = 0, end = 0, spent = 0;

    /* Find the index of the element */
#pragma omp parallel num_threads(threads) shared(array, count, spent) //private(index, found)
    {
        double begin = omp_get_wtime();
#pragma omp for
        for (int i = 0; i < count; i++) {
            if (found) {
                i = count;
            } else if (array[i] == target) {
                index = i;
                found = true;
            }
        }

        // Timing
        end = omp_get_wtime();
        spent = (double) (end - begin);
    }
//    printf("Found occurence of %d at index %d;\n", target, index);
    free(array);
    return (spent);
}

int main(int argc, char **argv) {
    int count = 10000000;     ///< Number of array elements
//    int count = 1162267;
    int threads = 1;            ///< Number of treads
    int repeats = 20;

    double aver_spent = 0.0;

    /* Determine the OpenMP support */
    printf("OpenMP: %d;\n======\n", _OPENMP);
    printf("Threads amount %d\n", omp_get_num_procs());

    for (threads = 1; threads <= 10; ++threads) {
        for (int i = 0; i < repeats; ++i) {
            aver_spent += seek_once(count, threads);
        }
        aver_spent /= repeats;
        printf("%d\t%lf\n", threads, aver_spent);
    }
}

