#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

double time_find_max(const int *array, const int threads, const int count) {
    int max = -1;
    double start_time = omp_get_wtime();
#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) default(none)
    {
#pragma omp for schedule(auto)
        for (int i = 0; i < count; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

//int comp_find_max(const int *array, const int threads, const int count) {
//    int max = -1;
//    int counter = 0;
////    double start_time = omp_get_wtime();
//#pragma omp parallel num_threads(threads) shared(array, count) reduction(max: max) reduction(+: counter) default(none)
//    {
//#pragma omp for
//        for (int i = 0; i < count; i++) {
//            if (array[i] > max) {
//                max = array[i];
//            }
//            counter++;
//        }
//    }
////    double end_time = omp_get_wtime();
//    return counter;
//}
// calculate for threads
int main(int argc, char **argv) {
    const int count = pow(10, 8);     ///< Number of array elements
    const int random_seed = 920215; ///< RNG seed
    const int threads = omp_get_num_procs();
    const int num_seed = 1;
    /* Determine the OpenMP support */
    printf("OpenMP: %d;\n======\n", _OPENMP);

    double *s = calloc(omp_get_num_procs(), sizeof(double));
    for (int seed_num = 0; seed_num < num_seed; seed_num++) {
        int *array = 0;                 ///< The array we need to find the max in
        /* Initialize the RNG */
        srand(random_seed + seed_num * 1024);

        /* Generate the random array */
        array = (int *) malloc(count * sizeof(int));
        for (int i = 0; i < count; i++) {
            array[i] = rand();
        }

        for (int n = 1; n <= threads; n++) {

            s[n - 1] += time_find_max(array, n, count);
        }
        printf("%d\n", seed_num);
        free(array);
    }

    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[i - 1] / num_seed);
    }
    printf("\n");
    for (int i = 1; i <= threads; i++) {
        printf("(%d,%lf)", i, s[0] / s[i - 1] / i);
    }
    printf("\n");
    for (int i = 1; i <= threads; i++) {
        printf("%lf ", s[i - 1] / num_seed);
    }
    free(s);
    return 0;
}

//(1,0.073284)(2,0.059692)(3,0.045171)(4,0.041593)(5,0.079401)(6,0.061941)(7,0.060969)(8,0.061660)(9,0.069163)(10,0.076433)
//(1,1.000000)(2,0.613843)(3,0.540788)(4,0.440478)(5,0.184592)(6,0.197187)(7,0.171711)(8,0.148564)(9,0.117731)(10,0.095880)
//0.073284 0.059692 0.045171 0.041593 0.079401 0.061941 0.060969 0.061660 0.069163 0.076433

//(1,0.066659)(2,0.038188)(3,0.029404)(4,0.022628)(5,0.019480)(6,0.016224)(7,0.014158)(8,0.012643)(9,0.012482)(10,0.012489)
//(1,1.000000)(2,0.872779)(3,0.755664)(4,0.736478)(5,0.684371)(6,0.684773)(7,0.672586)(8,0.659047)(9,0.593362)(10,0.533728)
//0.066659 0.038188 0.029404 0.022628 0.019480 0.016224 0.014158 0.012643 0.012482 0.012489

//(1,0.065181)(2,0.035094)(3,0.023839)(4,0.017896)(5,0.014454)(6,0.012062)(7,0.010668)(8,0.009560)(9,0.009175)(10,0.009011)
//(1,1.000000)(2,0.928658)(3,0.911391)(4,0.910542)(5,0.901884)(6,0.900667)(7,0.872872)(8,0.852274)(9,0.789346)(10,0.723338)
//0.065181 0.035094 0.023839 0.017896 0.014454 0.012062 0.010668 0.009560 0.009175 0.009011

//calc for one thread

//int main(int argc, char **argv) {
//    const int random_seed = 920215; ///< RNG seed
//    const int threads = omp_get_num_procs();
//    const int num_seed = 15;
//    /* Determine the OpenMP support */
//    printf("OpenMP: %d;\n======\n", _OPENMP);
//    for (int c = 1; c <= 6; c++) {
//        const int count = pow(10, c);
//        long long s = 0;
//        for (int seed_num = 0; seed_num < num_seed; seed_num++) {
//            int *array = 0;                 ///< The array we need to find the max in
//            /* Initialize the RNG */
//            srand(random_seed + seed_num * 1024);
//
//            /* Generate the random array */
//            array = (int *) malloc(count * sizeof(int));
//            for (int i = 0; i < count; i++) {
//                array[i] = rand();
//            }
//
//            s += comp_find_max(array, 1, count);
////            printf("%d\n", seed_num);
//            free(array);
//        }
//
//        printf("(%d,%d)\n", count, s / num_seed);
//    }
//    for (int c = 1; c <= 20; c++) {
//        const int count = 5 * c * pow(10, 6);
//        long long s = 0;
//        for (int seed_num = 0; seed_num < num_seed; seed_num++) {
//            int *array = 0;                 ///< The array we need to find the max in
//            /* Initialize the RNG */
//            srand(random_seed + seed_num * 1024);
//
//            /* Generate the random array */
//            array = (int *) malloc(count * sizeof(int));
//            for (int i = 0; i < count; i++) {
//                array[i] = rand();
//            }
//
//            s += comp_find_max(array, 1, count);
////            printf("%d\n", seed_num);
//            free(array);
//        }
//
//        printf("(%d,%d)\n", count, s / num_seed);
//    }
//    return 0;
//}
//(10000,0.000007)(100000,0.000085)(1000000,0.000672)(5000000,0.003306)(10000000,0.006522)(15000000,0.009736)(20000000,0.013044)(25000000,0.016384)(30000000,0.019665)(35000000,0.022910)(40000000,0.026806)(45000000,0.029656)(50000000,0.032722)(55000000,0.035976)(60000000,0.039364)(65000000,0.042488)(70000000,0.046016)(75000000,0.049158)(80000000,0.052471)(85000000,0.055796)(90000000,0.058934)(95000000,0.062085)(100000000,0.065612)

//
//(10000,10000)(100000,100000)(1000000,1000000)(5000000,5000000)(10000000,10000000)(15000000,15000000)(20000000,20000000)(25000000,25000000)(30000000,30000000)(35000000,35000000)(40000000,40000000)(45000000,45000000)(50000000,50000000)(55000000,55000000)(60000000,60000000)(65000000,65000000)(70000000,70000000)(75000000,75000000)(80000000,80000000)(85000000,85000000)(90000000,90000000)(95000000,95000000)(100000000,100000000)