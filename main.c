#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <unistd.h>
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
int is_prime(int n) {
    int cnt = 0;
    if (n <= 1) {
        return 0;
    } else {
        for (int i = 1; i <= n; i++) {
            if (n % i == 0)
                cnt++;
        }
        if (cnt > 2) {
            return 0;
        } else {
            return 1;
        }
    }
}

void print_stats(const double *s, const int threads) {
    for (int i = 1; i <= threads; i++) {
        printf("%d & %lf & %lf & %lf \\\\ \\hline \n", i, s[i - 1], s[0] / s[i - 1], s[0] / s[i - 1] / i);
    }
}

int main() {
//    double s[10] = {0.066684, 0.033366, 0.022494, 0.016973, 0.014038, 0.011782, 0.010492, 0.009651, 0.011957, 0.014164};
//    print_stats(s, 10);
    long count = 1e7;
    int size = 6;
    long tmp = count / size;
    long s1 = 0;
    long s2 = 0;
    long *counts = malloc(size * sizeof(long));
    long *displs = malloc(size * sizeof(long));
    for (int i = 0; i < size; i++) {
        counts[i] = (long) (i + 1) * count / size - (long) i * count / size;
        displs[i] = (long) i * count / size;
    }
    for (int i = 0; i < size; i++) {
        s1 += displs[i];
        s2 += counts[i];
    }
    printf("dsfg");
//    s={};
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
    return 0;
}
