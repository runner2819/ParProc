#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <unistd.h>
#include <mpi.h>

void generate_array(long *array, const int count, const int seed) {
    srand(seed);
    for (int i = 0; i < count; i++) {
        array[i] = i;
    }
}

int main(int argc, char **argv) {
    int ret = -1;    ///< For return values
    int size = -1;    ///< Total number of processors
    int rank = -1;    ///< This processor's number
    int divide = 840;
    const int count = 1e4;
    const long random_seed = 920215;
    const int num_seed = 30;

    int part_size = (count + divide - 1) / divide;

    ret = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;


    long *array;
    int *counts = malloc(divide * sizeof(int));
    int *displs = malloc(divide * sizeof(int));
    int offset = 0;
    for (int i = 0; i < divide; i++) {
        counts[i] = (long) (i + 1) * count / divide - (long) i * count / divide;
        displs[i] = offset;
        offset += counts[i];
    }


    if (!rank) {
        array = malloc(count * sizeof(long));
    }

    if (!rank) {
        generate_array(array, count, random_seed + random_seed * 1024);
    }
    long *chunk = malloc(part_size * sizeof(long));

    MPI_Scatterv(array, counts, displs, MPI_LONG, chunk, part_size, MPI_LONG, 0, MPI_COMM_WORLD);

    printf("%d\n", chunk[0]);

    free(chunk);

    free(counts);
    free(displs);
    if (!rank) {
        free(array);
    }
    ret = MPI_Finalize();
    return (0);
}

//int main() {
//    const int count = 100;
//    const int threads = omp_get_num_procs();
//    int *array = malloc(count * sizeof(int));
//    for (int i = 0; i < count; i++) {
//        array[i] = i;
//    }
//    int *counters = calloc(threads, sizeof(int));
//#pragma omp parallel num_threads(threads) shared(array, count, counters) default(none)
//    {
//#pragma omp for ordered
//        for (int i = 0; i < count; i++) {
//#pragma omp ordered
//            printf("Thread %d is running number %d\n", omp_get_thread_num(), array[i]);
//        }
//    }
//    return 0;
//}
//omp_shed_t

//void cloning(int *ar, int count, int *clone) {
//    int n = omp_get_num_procs();
//#pragma omp parallel for
//    for (int i = 0; i < n; i++) {
//        int t = i * count / n;
//        int tt = (i + 1) * count / n;
//        memcpy(clone + t, ar + t, (tt - t) * sizeof(int));
//    }
//}
//int is_prime(int n) {
//    int cnt = 0;
//    if (n <= 1) {
//        return 0;
//    } else {
//        for (int i = 1; i <= n; i++) {
//            if (n % i == 0)
//                cnt++;
//        }
//        if (cnt > 2) {
//            return 0;
//        } else {
//            return 1;
//        }
//    }
//}

//void print_stats(const double *s, const int threads, const int num_seed) {
//    printf("Time:\n\\def \\time{");
//    for (int i = 1; i <= threads; i++) {
//        printf("(%d,%lf)", i, s[i - 1] / num_seed);
//    }
//    printf("}");
//    printf("\nAcceleration:\n\\def \\acc{");
//    for (int i = 1; i <= threads; i++) {
//        printf("(%d,%lf)", i, s[0] / s[i - 1]);
//    }
//    printf("}");
//    printf("\nEfficiency:\n\\def \\eff{");
//    for (int i = 1; i <= threads; i++) {
//        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
//    }
//    printf("}\n");
//}
//
//int main() {
//    double s[8] = {44.939674, 20.137259, 11.645667, 10.627241, 7.666202, 6.091503, 5.974657, 5.368062};
//    print_stats(s, 8, 1);
//    unsigned long long t = 1;
//    for (int i = 0; i < 100; i++) {
//        if (is_prime(i)) {
//            t *= i;
//        }
//    }
//    printf("%lld", t);
//    printf("%d %d %d %d", sizeof(int), sizeof(long), sizeof(long int), sizeof(long long));
//    int count = pow(10, 5)+10;
//    int *ar = malloc(count * sizeof(int));
//    int *ar2 = malloc(count * sizeof(int));
//#pragma omp parallel for shared(ar, ar2, count) default(none)
//    for (int i = 0; i < count; i++) {
//        ar[i] = i;
//    }
//
//    cloning(ar, count, ar2);
//#pragma omp parallel for shared(ar2, count) default(none)
//    for (int i = 0; i < count; i++) {
//        if (ar2[i] != i) {
//            printf("PIZDEC on %d", i);
//        }
//    }
//    omp_set_schedule()
//    printf("%d", omp_sched_static);
//    for (int i = 0; i < 10; i++) {
//        system("echo \"\a\"");
//    }
//return 0;
//}
