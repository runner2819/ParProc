/*
 * pso_global_vs_local.c
 * Сравнение сходимости глобального и локального (кольцо) PSO
 * на функции Растригина (мультимодальная).
 * Вывод: iteration, global_best_fitness_gbest, global_best_fitness_lbest
 *
 * Компиляция: gcc -O2 -fopenmp -o pso_gl pso_global_vs_local.c -lm
 * Запуск:     ./pso_gl > convergence.csv
 * График:     X = итерация, Y = лучшее значение фитнеса
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define NUM_PARTICLES   30
#define NUM_DIMS   10
#define MAX_ITER      20000
#define X_MIN        -5.12
#define X_MAX         5.12
#define C1            1.5
#define C2            1.5
#define W_START       0.9
#define W_END         0.4
#define RING_NEIGHBORS 2  /* по 1 соседу с каждой стороны */

/* Функция Растригина — мультимодальная, глобальный минимум = 0 в (0,...,0) */
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
    double pos[NUM_DIMS];
    double vel[NUM_DIMS];
    double pbest_pos[NUM_DIMS];
    double pbest_val;
} Particle;

/* Инициализация роя */
void init_swarm(Particle *swarm, int n, unsigned int *seeds) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        unsigned int *seed = &seeds[omp_get_thread_num()];
        for (int j = 0; j < NUM_DIMS; j++) {
            swarm[i].pos[j] = rand_range(X_MIN, X_MAX, seed);
            swarm[i].vel[j] = 0.0;
            swarm[i].pbest_pos[j] = swarm[i].pos[j];
        }
        swarm[i].pbest_val = rastrigin(swarm[i].pos, NUM_DIMS);
    }
}

/* Копирование роя */
void copy_swarm(Particle *dst, const Particle *src, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

/* Глобальный PSO — одна итерация */
double pso_global_step(Particle *swarm, int n, double *gbest_pos,
                       double gbest_val, double w, unsigned int *seeds) {
    /* Обновить скорости и позиции */
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        unsigned int *seed = &seeds[omp_get_thread_num()];
        for (int j = 0; j < NUM_DIMS; j++) {
            double r1 = rand_range(0, 1, seed);
            double r2 = rand_range(0, 1, seed);
            swarm[i].vel[j] = w * swarm[i].vel[j]
                              + C1 * r1 * (swarm[i].pbest_pos[j] - swarm[i].pos[j])
                              + C2 * r2 * (gbest_pos[j] - swarm[i].pos[j]);
            swarm[i].pos[j] += swarm[i].vel[j];
            /* Ограничение позиции */
            if (swarm[i].pos[j] < X_MIN) swarm[i].pos[j] = X_MIN;
            if (swarm[i].pos[j] > X_MAX) swarm[i].pos[j] = X_MAX;
        }
        double val = rastrigin(swarm[i].pos, NUM_DIMS);
        if (val < swarm[i].pbest_val) {
            swarm[i].pbest_val = val;
            for (int j = 0; j < NUM_DIMS; j++)
                swarm[i].pbest_pos[j] = swarm[i].pos[j];
        }
    }
    /* Обновить глобальный лучший */
    for (int i = 0; i < n; i++) {
        if (swarm[i].pbest_val < gbest_val) {
            gbest_val = swarm[i].pbest_val;
            for (int j = 0; j < NUM_DIMS; j++)
                gbest_pos[j] = swarm[i].pbest_pos[j];
        }
    }
    return gbest_val;
}

/* Локальный PSO (кольцо) — одна итерация */
double pso_local_step(Particle *swarm, int n, double *gbest_pos,
                      double gbest_val, double w, unsigned int *seeds) {
    /* Для каждой частицы определить лучшего соседа по кольцу */
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        unsigned int *seed = &seeds[omp_get_thread_num()];
        /* Найти лучшего в окрестности {i-1, i, i+1} */
        int left = (i - 1 + n) % n;
        int right = (i + 1) % n;
        double lbest_val = swarm[i].pbest_val;
        int lbest_idx = i;
        if (swarm[left].pbest_val < lbest_val) {
            lbest_val = swarm[left].pbest_val;
            lbest_idx = left;
        }
        if (swarm[right].pbest_val < lbest_val) {
            lbest_val = swarm[right].pbest_val;
            lbest_idx = right;
        }

        for (int j = 0; j < NUM_DIMS; j++) {
            double r1 = rand_range(0, 1, seed);
            double r2 = rand_range(0, 1, seed);
            swarm[i].vel[j] = w * swarm[i].vel[j]
                              + C1 * r1 * (swarm[i].pbest_pos[j] - swarm[i].pos[j])
                              + C2 * r2 * (swarm[lbest_idx].pbest_pos[j] - swarm[i].pos[j]);
            swarm[i].pos[j] += swarm[i].vel[j];
            if (swarm[i].pos[j] < X_MIN) swarm[i].pos[j] = X_MIN;
            if (swarm[i].pos[j] > X_MAX) swarm[i].pos[j] = X_MAX;
        }
        double val = rastrigin(swarm[i].pos, NUM_DIMS);
        if (val < swarm[i].pbest_val) {
            swarm[i].pbest_val = val;
            for (int j = 0; j < NUM_DIMS; j++)
                swarm[i].pbest_pos[j] = swarm[i].pos[j];
        }
    }
    /* Обновить глобальный лучший */
    for (int i = 0; i < n; i++) {
        if (swarm[i].pbest_val < gbest_val) {
            gbest_val = swarm[i].pbest_val;
            for (int j = 0; j < NUM_DIMS; j++)
                gbest_pos[j] = swarm[i].pbest_pos[j];
        }
    }
    return gbest_val;
}

int main(void) {
    int num_threads;
#pragma omp parallel
    {
        num_threads = omp_get_num_threads();
    }

    unsigned int *seeds = malloc(num_threads * sizeof(unsigned int));
    for (int i = 0; i < num_threads; i++)
        seeds[i] = 42 + i * 1000;

    /* Два роя с одинаковой инициализацией */
    Particle *swarm_g = malloc(NUM_PARTICLES * sizeof(Particle));
    Particle *swarm_l = malloc(NUM_PARTICLES * sizeof(Particle));

    init_swarm(swarm_g, NUM_PARTICLES, seeds);
    copy_swarm(swarm_l, swarm_g, NUM_PARTICLES);

    /* Глобальные лучшие */
    double gbest_pos_g[NUM_DIMS], gbest_pos_l[NUM_DIMS];
    double gbest_val_g = DBL_MAX, gbest_val_l = DBL_MAX;

    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (swarm_g[i].pbest_val < gbest_val_g) {
            gbest_val_g = swarm_g[i].pbest_val;
            for (int j = 0; j < NUM_DIMS; j++)
                gbest_pos_g[j] = swarm_g[i].pbest_pos[j];
        }
    }
    gbest_val_l = gbest_val_g;
    for (int j = 0; j < NUM_DIMS; j++)
        gbest_pos_l[j] = gbest_pos_g[j];

    printf("iteration,global_pso,local_pso_ring\n");
    printf("0,%.6f,%.6f\n", gbest_val_g, gbest_val_l);

    for (int t = 1; t <= MAX_ITER; t++) {
        double w = W_START - (W_START - W_END) * (double) t / MAX_ITER;

        gbest_val_g = pso_global_step(swarm_g, NUM_PARTICLES,
                                      gbest_pos_g, gbest_val_g, w, seeds);
        gbest_val_l = pso_local_step(swarm_l, NUM_PARTICLES,
                                     gbest_pos_l, gbest_val_l, w, seeds);

        printf("%d,%.6f,%.6f\n", t, gbest_val_g, gbest_val_l);
    }

    free(swarm_g);
    free(swarm_l);
    free(seeds);
    return 0;
}
