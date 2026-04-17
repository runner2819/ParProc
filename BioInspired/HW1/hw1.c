#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>


#define A_LEFT    -10.0
#define A_RIGHT   10.0
#define CHROM_LEN 15
#define MAX_VAL   ((1 << CHROM_LEN) - 1)

#define DEFAULT_POP_SIZE 100
#define DEFAULT_MAX_GEN  500
#define DEFAULT_PC       0.85
#define DEFAULT_PM       0.1
#define TOURNAMENT_SIZE  3

#define DEFAULT_ACCURACY   (A_RIGHT - A_LEFT) / (double) MAX_VAL
#define STAGNATION_LIMIT   10

double fit_f(double x) {
    return (x - 1.0) * cos(3.0 * x - 15.0);
}

double decode(int chromo) {
    return A_LEFT + (double) chromo * (A_RIGHT - A_LEFT) / MAX_VAL;
}

int rand_int(int max) {
    return rand() % max;
}

double rand_double(void) {
    return (double) rand() / (RAND_MAX + 1.0);
}

typedef struct {
    double best_x, best_f, elapsed;
    int generations;
} GAResult;

GAResult run_ga(int population_size, int max_gen, double pc, double pm, double accuracy, int stagnation_limit,
                int verbose) {
    GAResult result;
    int *pop = malloc(population_size * sizeof(int));
    int *next = malloc(population_size * sizeof(int));
    double *fit = malloc(population_size * sizeof(double));

    for (int i = 0; i < population_size; i++)
        pop[i] = rand_int(MAX_VAL + 1);

    double global_best_f = -1e30, global_best_x = 0;
    double prev_gen_best_f = -1e30;
    int stagnation_count = 0;
    double t_start = omp_get_wtime();
    int actual_gen = 0;

    for (int gen = 0; gen < max_gen; gen++) {
        for (int i = 0; i < population_size; i++)
            fit[i] = fit_f(decode(pop[i]));

        int bi = 0;
        for (int i = 1; i < population_size; i++)
            if (fit[i] > fit[bi]) bi = i;

        double current_gen_best_f = fit[bi];

        if (current_gen_best_f > global_best_f) {
            global_best_f = current_gen_best_f;
            global_best_x = decode(pop[bi]);
        }

        if (verbose)
            printf("Поколение %d | глоб. макс: f = %.6f  x = %.6f\n",
                   gen, global_best_f, global_best_x);

        if (gen > 0) {
            double delta = fabs(current_gen_best_f - prev_gen_best_f);
            if (delta < accuracy) {
                stagnation_count++;
            } else {
                stagnation_count = 0;
            }
        }
        prev_gen_best_f = current_gen_best_f;
        actual_gen = gen + 1;

        if (stagnation_count >= stagnation_limit) {
            // stop_reason = STOP_STAGNATION;
            if (verbose)
                printf("\n>>> ОСТАНОВКА: стагнация %d поколений "
                       "(изменение fitness < %.2e)\n",
                       stagnation_limit, accuracy);
            break;
        }
        /* Элитизм */
        next[0] = pop[bi];

        /* Генерация потомков */
        for (int i = 1; i < population_size; i += 2) {
            /* Турнирная селекция */
            int p1 = rand_int(population_size), p2 = rand_int(population_size);
            for (int t = 1; t < TOURNAMENT_SIZE; t++) {
                int r = rand_int(population_size);
                if (fit[r] > fit[p1]) p1 = r;
                r = rand_int(population_size);
                if (fit[r] > fit[p2]) p2 = r;
            }

            int c1 = pop[p1], c2 = pop[p2];

            /* Кроссинговер */
            if (rand_double() < pc) {
                int point = 1 + rand_int(CHROM_LEN - 1);
                int mask = (1 << point) - 1;
                c1 = (pop[p1] & ~mask) | (pop[p2] & mask);
                c2 = (pop[p2] & ~mask) | (pop[p1] & mask);
            }

            /* Мутация */
            for (int b = 0; b < CHROM_LEN; b++) {
                if (rand_double() < pm) c1 ^= (1 << b);
                if (rand_double() < pm) c2 ^= (1 << b);
            }
            c1 &= MAX_VAL;
            c2 &= MAX_VAL;

            next[i] = c1;
            if (i + 1 < population_size) next[i + 1] = c2;
        }

        /* Смена поколений */
        int *tmp = pop;
        pop = next;
        next = tmp;
    }

    result.best_x = global_best_x;
    result.best_f = global_best_f;
    result.generations = actual_gen;
    result.elapsed = omp_get_wtime() - t_start;

    free(pop);
    free(next);
    free(fit);
    return result;
}

#define RESEARCH_RUNS 20

GAResult run_ga_avg(int population_size, int max_gen, double pc, double pm) {
    GAResult avg = {0};
    for (int r = 0; r < RESEARCH_RUNS; r++) {
        GAResult cur = run_ga(population_size, max_gen, pc, pm, DEFAULT_ACCURACY,STAGNATION_LIMIT, 0);
        avg.best_x += cur.best_x;
        avg.best_f += cur.best_f;
        avg.elapsed += cur.elapsed;
        avg.generations += cur.generations;
    }
    avg.best_x /= RESEARCH_RUNS;
    avg.best_f /= RESEARCH_RUNS;
    avg.elapsed /= RESEARCH_RUNS;
    avg.generations /= RESEARCH_RUNS;
    return avg;
}

void research_pop_size(void) {
    printf("\n=== Зависимость от размера популяции (pc=%.2f, pm=%.4f, gen=%d) ===\n",
           DEFAULT_PC, DEFAULT_PM, DEFAULT_MAX_GEN);
    printf("%s %12s %12s %12s %10s\n", "Pop", "Best x", "Best f(x)", "Time(s)", "Avg Gen");
    for (int size = 10; size <= 1.5 * DEFAULT_POP_SIZE; size += 10) {
        GAResult r = run_ga_avg(size, DEFAULT_MAX_GEN, DEFAULT_PC, DEFAULT_PM);
        printf("%d %.6f %.4f %d\n", size, r.best_f, r.elapsed, r.generations);
    }
}

void research_crossover(void) {
    printf("\n=== Зависимость от вероятности кроссинговера (pop=%d, pm=%.4f, gen=%d) ===\n",
           DEFAULT_POP_SIZE, DEFAULT_PM, DEFAULT_MAX_GEN);
    printf("%s %12s %12s %12s %10s\n", "Pc", "Best x", "Best f(x)", "Time(s)", "Avg Gen");
    double pcs[] = {0.0, 0.1, 0.3, 0.5, 0.7, 0.85, 0.95, 1.0};
    for (int i = 0; i < 8; i++) {
        GAResult r = run_ga_avg(DEFAULT_POP_SIZE, DEFAULT_MAX_GEN, pcs[i], DEFAULT_PM);
        printf("%.2f %.6f %.4f %d\n", pcs[i], r.best_f, r.elapsed, r.generations);
    }
}

void research_mutation(void) {
    printf("\n=== Зависимость от вероятности мутации (pop=%d, pc=%.2f, gen=%d) ===\n",
           DEFAULT_POP_SIZE, DEFAULT_PC, DEFAULT_MAX_GEN);
    printf("%s %12s %12s %12s %10s\n", "Pm", "Best x", "Best f(x)", "Time(s)", "Avg Gen");
    double pms[] = {0.0, 0.001, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2};
    for (int i = 0; i < 8; i++) {
        GAResult r = run_ga_avg(DEFAULT_POP_SIZE, DEFAULT_MAX_GEN, DEFAULT_PC, pms[i]);
        printf("%.4f %.6f %.4f %d\n", pms[i], r.best_f, r.elapsed, r.generations);
    }
}

int main(void) {
    srand((unsigned) time(NULL));

    printf("f(x) = (x-1)*cos(3x-15), x in [%.0f, %.0f]\n", A_LEFT, A_RIGHT);
    printf("Хромосома: %d бит, точность: %.2e\n", CHROM_LEN, (A_RIGHT - A_LEFT) / (double) MAX_VAL);

    /* Основной запуск с выводом каждого поколения */
    GAResult r = run_ga(DEFAULT_POP_SIZE, DEFAULT_MAX_GEN, DEFAULT_PC, DEFAULT_PM, DEFAULT_ACCURACY,STAGNATION_LIMIT,
                        1);
    printf("\n>>> ИТОГ: x = %.6f, f(x) = %.6f, время = %.4f с\n", r.best_x, r.best_f, r.elapsed);

    /* Исследование параметров */
    research_pop_size();
    research_crossover();
    research_mutation();

    return 0;
}
