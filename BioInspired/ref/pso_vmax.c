/*
 * pso_vmax.c
 * Влияние ограничения максимальной скорости на сходимость PSO.
 * Тестовая функция: Растригина 10D.
 * Варианты Vmax: 0.1*(xmax-xmin), 0.3*, 0.5*, 1.0*, без ограничения
 *
 * Компиляция: gcc -O2 -fopenmp -o pso_vmax pso_vmax.c -lm
 * Запуск:     ./pso_vmax > vmax.csv
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>


#define NUM_PARTICLES   30
#define NUM_DIMS   10
#define ITER 400
#define X_MIN -5.12
#define X_MAX 5.12
#define C1   1.0
#define C2   1.0
#define RANGE (X_MAX - X_MIN)

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

void run_pso_vmax(double vmax_frac, unsigned int bseed, double *log_buf) {
    /* vmax_frac < 0 означает без ограничения */
    double vmax = (vmax_frac >= 0) ? vmax_frac * RANGE : 1e30;
    Part sw[NUM_PARTICLES];
    int nt;
#pragma omp parallel
    {
        nt = omp_get_num_threads();
    }
    unsigned int se[16];
    for (int i = 0; i < nt && i < 16; i++) se[i] = bseed + i * 431;

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

    double gb[NUM_DIMS], gv = DBL_MAX;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (sw[i].pb_val < gv) {
            gv = sw[i].pb_val;
            for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
        }
    }
    log_buf[0] = gv;

    for (int t = 1; t <= ITER; t++) {
        double w = 0.9 - 0.5 * (double) t / ITER;

#pragma omp parallel for
        for (int i = 0; i < NUM_PARTICLES; i++) {
            unsigned int *s = &se[omp_get_thread_num()];
            for (int j = 0; j < NUM_DIMS; j++) {
                double r1 = rand_range(0, 1, s), r2 = rand_range(0, 1, s);
                sw[i].v[j] = w * sw[i].v[j]
                             + C1 * r1 * (sw[i].pb[j] - sw[i].x[j])
                             + C2 * r2 * (gb[j] - sw[i].x[j]);
                /* Ограничение скорости */
                if (sw[i].v[j] > vmax) sw[i].v[j] = vmax;
                if (sw[i].v[j] < -vmax) sw[i].v[j] = -vmax;

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
            if (sw[i].pb_val < gv) {
                gv = sw[i].pb_val;
                for (int j = 0; j < NUM_DIMS; j++) gb[j] = sw[i].pb[j];
            }
        }
        log_buf[t] = gv;
    }
}

int main(void) {
    double fracs[] = {0.1, 0.3, 0.5, 1.0, -1.0};
    const char *labels[] = {
        "vmax_10pct", "vmax_30pct", "vmax_50pct",
        "vmax_100pct", "no_limit"
    };
    int nv = 5;
    double *logs[5];

    for (int k = 0; k < nv; k++) {
        logs[k] = malloc((ITER + 1) * sizeof(double));
        run_pso_vmax(fracs[k], 42, logs[k]);
    }

    printf("iteration");
    for (int k = 0; k < nv; k++) printf(",%s", labels[k]);
    printf("\n");

    for (int t = 0; t <= ITER; t++) {
        printf("%d", t);
        for (int k = 0; k < nv; k++)
            printf(",%.6e", logs[k][t]);
        printf("\n");
    }

    for (int k = 0; k < nv; k++) free(logs[k]);
    return 0;
}
