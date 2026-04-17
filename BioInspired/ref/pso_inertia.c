/*
 * pso_inertia.c
 * Влияние стратегии изменения веса инерции на сходимость PSO.
 * Тестовая функция: сфера f(x) = sum(x_i^2)
 *
 * Компиляция: gcc -O2 -fopenmp -o pso_inertia pso_inertia.c -lm
 * Запуск:     ./pso_inertia > inertia.csv
 * График:     X = итерация, Y = лучшее значение фитнеса (лог. шкала)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define NUM_PARTICLES   30
#define NUM_DIMS   10
#define ITER 20000
#define X_MIN -5.12
#define X_MAX 5.12
#define C1   1.6

#define C2   1.6

// double sphere(const double *x, int n) {
//     double s = 0;
//     for (int i = 0; i < n; i++) s += x[i] * x[i];
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
} P;

void init(P *sw, unsigned int *se) {
#pragma omp parallel for
    for (int i = 0; i < NUM_PARTICLES; i++) {
        unsigned int *s = &se[omp_get_thread_num()];
        for (int j = 0; j < NUM_DIMS; j++) {
            sw[i].x[j] = rand_range(X_MIN, X_MAX, s);
            sw[i].v[j] = 0;
            sw[i].pb[j] = sw[i].x[j];
        }
        sw[i].pb_val = rastrigin(sw[i].x, NUM_DIMS);
    }
}

double run_pso(P *sw, int mode, unsigned int *se, double *log_buf) {
    /* mode: 0=const w=0.7, 1=linear 0.9->0.4, 2=nonlinear */
    double gb[NUM_DIMS], gb_val = DBL_MAX;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (sw[i].pb_val < gb_val) {
            gb_val = sw[i].pb_val;
            for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
        }
    }

    log_buf[0] = gb_val;

    for (int t = 1; t <= ITER; t++) {
        double w;
        switch (mode) {
            case 0: w = 0.7;
                break;
            case 1: w = 0.9 - (0.9 - 0.4) * (double) t / ITER;
                break;
            case 2: /* w(t+1) = alpha * w(t), alpha=0.99 */
                w = 0.9 * pow(0.99, t);
                break;
            default: w = 0.7;
        }

#pragma omp parallel for
        for (int i = 0; i < NUM_PARTICLES; i++) {
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
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (sw[i].pb_val < gb_val) {
                gb_val = sw[i].pb_val;
                for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
            }
        }
        log_buf[t] = gb_val;
    }
    return gb_val;
}

int main(void) {
    int nt;
#pragma omp parallel
    {
        nt = omp_get_num_threads();
    }
    unsigned int *se = malloc(nt * sizeof(unsigned int));
    for (int i = 0; i < nt; i++) se[i] = 123 + i * 777;

    double log_const[ITER + 1], log_lin[ITER + 1], log_nonlin[ITER + 1];

    /* Общая начальная инициализация */
    P *base = malloc(NUM_PARTICLES * sizeof(P));
    P *sw = malloc(NUM_PARTICLES * sizeof(P));
    init(base, se);

    /* Режим 0: постоянный w */
    for (int i = 0; i < NUM_PARTICLES; i++) sw[i] = base[i];
    /* Сбросить seeds для воспроизводимости */
    for (int i = 0; i < nt; i++) se[i] = 200 + i * 333;
    run_pso(sw, 0, se, log_const);

    /* Режим 1: линейный */
    for (int i = 0; i < NUM_PARTICLES; i++) sw[i] = base[i];
    for (int i = 0; i < nt; i++) se[i] = 200 + i * 333;
    run_pso(sw, 1, se, log_lin);

    /* Режим 2: нелинейный */
    for (int i = 0; i < NUM_PARTICLES; i++) sw[i] = base[i];
    for (int i = 0; i < nt; i++) se[i] = 200 + i * 333;
    run_pso(sw, 2, se, log_nonlin);

    printf("iteration,w_const_0.7,w_linear,w_nonlinear\n");
    for (int t = 0; t <= ITER; t++) {
        printf("%d,%.6f,%.6f,%.6f\n", t,
               log_const[t], log_lin[t], log_nonlin[t]);
    }

    free(base);
    free(sw);
    free(se);
    return 0;
}
