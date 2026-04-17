/*
 * pso_swarm_size.c
 * Влияние размера роя на качество и скорость сходимости.
 * Функция: Розенброка f(x) = sum[ 100*(x_{i+1}-x_i^2)^2 + (1-x_i)^2 ]
 * Глобальный минимум = 0 в (1,1,...,1)
 *
 * Компиляция: gcc -O2 -fopenmp -o pso_size pso_swarm_size.c -lm
 * Запуск:     ./pso_size > swarm_size.csv
 * График:     X = итерация, Y = лучший фитнес, серии по размеру роя
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define NUM_DIMS   10
#define ITER 20000
#define X_MIN -5.12
#define X_MAX 5.12
#define C1   1.5
#define C2   1.5
#define W0   0.9
#define WF   0.4

// double rosenbrock(const double *x, int n) {
//     double s = 0;
//     for (int i = 0; i < n - 1; i++) {
//         double a = x[i + 1] - x[i] * x[i];
//         double b = 1.0 - x[i];
//         s += 100.0 * a * a + b * b;
//     }
//     return s;
// }

double rastrigin(const double *x, int n) {
    double sum = 10.0 * n;
    for (int i = 0; i < n; i++) {
        sum += x[i] * x[i] - 10.0 * cos(2.0 * M_PI * x[i]);
    }
    return sum;
}

/* Случайное число в [lo, hi] */
double rand_range(double lo, double hi, unsigned int *seed) {
    return lo + (hi - lo) * ((double) rand_r(seed) / RAND_MAX);
}


typedef struct {
    double x[NUM_DIMS], v[NUM_DIMS], pb[NUM_DIMS], pb_val;
} Part;

double run_pso_n(int np, unsigned int base_seed, double *log_buf) {
    Part *sw = malloc(np * sizeof(Part));
    int nt;
#pragma omp parallel
    {
        nt = omp_get_num_threads();
    }
    unsigned int *se = malloc(nt * sizeof(unsigned int));
    for (int i = 0; i < nt; i++) se[i] = base_seed + i * 997;

    /* init */
#pragma omp parallel for
    for (int i = 0; i < np; i++) {
        unsigned int *s = &se[omp_get_thread_num()];
        for (int j = 0; j < NUM_DIMS; j++) {
            sw[i].x[j] = rand_range(X_MIN, X_MAX, s);
            sw[i].v[j] = 0;
            sw[i].pb[j] = sw[i].x[j];
        }
        sw[i].pb_val = rastrigin(sw[i].x, NUM_DIMS);
    }

    double gb[NUM_DIMS], gv = DBL_MAX;
    for (int i = 0; i < np; i++) {
        if (sw[i].pb_val < gv) {
            gv = sw[i].pb_val;
            for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
        }
    }
    log_buf[0] = gv;

    for (int t = 1; t <= ITER; t++) {
        double w = W0 - (W0 - WF) * (double) t / ITER;

#pragma omp parallel for
        for (int i = 0; i < np; i++) {
            unsigned int *s = &se[omp_get_thread_num()];
            for (int j = 0; j < NUM_DIMS; j++) {
                double r1 = rand_range(0, 1, s), r2 = rand_range(0, 1, s);
                sw[i].v[j] = w * sw[i].v[j]
                             + C1 * r1 * (sw[i].pb[j] - sw[i].x[j])
                             + C2 * r2 * (gb[j] - sw[i].x[j]);
                sw[i].x[j] += sw[i].v[j];
                if (sw[i].x[j] < X_MIN) sw[i].x[j] = X_MIN;
                if (sw[i].x[j] > X_MAX) sw[i].x[j] = X_MAX;
            }
            double val = rastrigin(sw[i].x, NUM_DIMS);
            if (val < sw[i].pb_val) {
                sw[i].pb_val = val;
                for (int j = 0; j < NUM_DIMS; j++) sw[i].pb[j] = sw[i].x[j];
            }
        }
        for (int i = 0; i < np; i++) {
            if (sw[i].pb_val < gv) {
                gv = sw[i].pb_val;
                for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
            }
        }
        log_buf[t] = gv;
    }

    free(sw);
    free(se);
    return gv;
}

int main(void) {
    int sizes[] = {10, 20, 30, 50, 100};
    int ns = 5;
    double *logs[5];
    for (int k = 0; k < ns; k++) {
        logs[k] = malloc((ITER + 1) * sizeof(double));
        run_pso_n(sizes[k], 42 + k * 10000, logs[k]);
    }

    printf("iteration,n10,n20,n30,n50,n100\n");
    for (int t = 0; t <= ITER; t++) {
        printf("%d", t);
        for (int k = 0; k < ns; k++)
            printf(",%.6f", logs[k][t]);
        printf("\n");
    }

    for (int k = 0; k < ns; k++) free(logs[k]);
    return 0;
}
