/*
 * pso_coefficients.c
 * Влияние соотношения c1/c2 на качество решения PSO.
 * Функция: Растригина (10D)
 * Варианты: (c1=3,c2=1), (c1=2,c2=2), (c1=1,c2=3), (c1=0,c2=2), (c1=2,c2=0)
 *
 * Компиляция: gcc -O2 -fopenmp -o pso_coeff pso_coefficients.c -lm
 * Запуск:     ./pso_coeff > coefficients.csv
 * График:     X = итерация, Y = лучший фитнес
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define NUM_PARTICLES   30
#define NUM_DIMS   10
#define ITER 20000
#define X_MIN  -5.12
#define X_MAX   5.12

double rastrigin(const double *x, int n) {
    double s = 10.0 * n;
    for (int i = 0; i < n; i++)
        s += x[i] * x[i] - 10.0 * cos(2.0 * M_PI * x[i]);
    return s;
}

/* Случайное число в [lo, hi] */
double rand_range(double lo, double hi, unsigned int *seed) {
    return lo + (hi - lo) * ((double) rand_r(seed) / RAND_MAX);
}


typedef struct {
    double x[NUM_DIMS], v[NUM_DIMS], pb[NUM_DIMS], pb_val;
} Part;

void run_pso(double c1, double c2, unsigned int bseed, double *log_buf) {
    Part sw[NUM_PARTICLES];
    int nt;
#pragma omp parallel
    {
        nt = omp_get_num_threads();
    }
    unsigned int se[16];
    for (int i = 0; i < nt && i < 16; i++) se[i] = bseed + i * 571;

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
                             + c1 * r1 * (sw[i].pb[j] - sw[i].x[j])
                             + c2 * r2 * (gb[j] - sw[i].x[j]);
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
    double c1s[] = {3.0, 2.0, 1.0, 0.0, 2.0};
    double c2s[] = {1.0, 2.0, 3.0, 2.0, 0.0};
    const char *labels[] = {
        "c1=3_c2=1", "c1=2_c2=2", "c1=1_c2=3",
        "c1=0_c2=2", "c1=2_c2=0"
    };
    int nv = 5;
    double *logs[5];

    for (int k = 0; k < nv; k++) {
        logs[k] = malloc((ITER + 1) * sizeof(double));
        run_pso(c1s[k], c2s[k], 42, logs[k]);
    }

    printf("iteration");
    for (int k = 0; k < nv; k++) printf(",%s", labels[k]);
    printf("\n");

    for (int t = 0; t <= ITER; t++) {
        printf("%d", t);
        for (int k = 0; k < nv; k++)
            printf(",%.6f", logs[k][t]);
        printf("\n");
    }

    for (int k = 0; k < nv; k++) free(logs[k]);
    return 0;
}
