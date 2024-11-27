#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdbool.h>

void create_array(int *array, const int count, const int random_seed) {
    srand(random_seed);

    for (int i = 0; i < count; i++) {
        array[i] = rand();
    }
    //array[10000] = 16;
}


double find_index_of_elem(int *array, const int count, const int target, const int threads) {
    int index = -1;
    double time_required = 0;
    atomic_bool found = false;
    int ccc = 0;
    double start = omp_get_wtime();

#pragma omp parallel num_threads(threads) shared(found, index) reduction(+: ccc) firstprivate(array, count, target) default(none)
    {
#pragma omp for
        for (int i = 0; i < count; i++) {
            if (found) i = count;
            if (!found && array[i] == target) {
#pragma omp critical
                {
                    index = i;
                    found = true;
                }
            }
            ccc++;
        }
    }

    double end = omp_get_wtime();
    time_required = end - start;
    printf("%d\n", ccc);
    //printf("Index is: %d\n", index);
    return time_required;
}


int main() {
    const int count = 100000;     ///< Number of array elements
    int random_seed = 920214; ///< RNG seed
    const int target = 256696242;          ///< Number to look for
    const int runs_amount = 10;
    double time_required = 0;

    int *array = (int *) malloc(count * sizeof(int));


    for (int i = 6; i <= 6; i++) {
        for (int k = 0; k < runs_amount; k++) {
            create_array(array, count, random_seed);
            time_required += find_index_of_elem(array, count, array[1000], i);
            random_seed += 6928;
        }

        printf("%lf ", (double) (time_required / runs_amount));

        random_seed = 920214;
        time_required = 0;
    }


    //printf("Found occurence of %d at index %d;\n", target, index);

    free(array);
    return (0);
}