#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

struct BruteRes {
    double t_first;
    double t_all;
    int num_sol;
};

struct GARes {
    double t_ga;
    int64_t min_fit;
    string reason;
    int gens;
};

struct Problem {
    int vec_idx;
    int64_t target;
    double dolya;
};

int64_t calc_fitness(uint32_t mask, const vector<int> &w, int64_t target, int n) {
    int64_t sum = 0;
    for (int j = 0; j < n; ++j) {
        if (mask & (1u << j)) {
            sum += w[j];
        }
    }
    return abs(sum - target);
}

BruteRes run_brute(const vector<int> &w, int64_t target, int n) {
    auto start = chrono::high_resolution_clock::now();
    int num_sol = 0;
    double t_first = -1.0;
    uint64_t total_sub = 1ull << n;
    for (uint64_t i = 0; i < total_sub; ++i) {
        int64_t sumv = 0;
        for (int j = 0; j < n; ++j) {
            if (i & (1ull << j)) {
                sumv += w[j];
            }
        }
        if (sumv == target) {
            if (t_first < 0.0) {
                auto now = chrono::high_resolution_clock::now();
                t_first = chrono::duration<double>(now - start).count();
            }
            ++num_sol;
        }
    }
    auto end = chrono::high_resolution_clock::now();
    double t_all = chrono::duration<double>(end - start).count();
    if (t_first < 0.0) t_first = t_all;
    return {t_first, t_all, num_sol};
}

using GAOut = tuple<int64_t, double, string, int>;

GAOut run_ga(const vector<int> &w, int64_t target, double max_time_sec, mt19937 &rng, int n) {
    const int POP_SIZE = 100;
    const double MUT_RATE = 1.0 / n;
    const int TOURNAMENT_SIZE = 3;
    const double CROSSOVER_RATE = 0.8;

    auto start_time = chrono::high_resolution_clock::now();

    vector<uint32_t> population(POP_SIZE);
    vector<int64_t> fitness(POP_SIZE);

    uint32_t all_bits = (1u << n) - 1u;

    // Инициализация популяции
    for (int i = 0; i < POP_SIZE; ++i) {
        population[i] = rng() & all_bits;
    }

    auto evaluate = [&](vector<uint32_t> &popu, vector<int64_t> &fits) -> int64_t {
        int64_t bestf = LLONG_MAX;
        for (int i = 0; i < POP_SIZE; ++i) {
            fits[i] = calc_fitness(popu[i], w, target, n);
            if (fits[i] < bestf) bestf = fits[i];
        }
        return bestf;
    };

    int64_t current_best = evaluate(population, fitness);
    int gen = 0;
    int stagnation_count = 0;

    while (true) {
        // Проверка условий остановки (текущее состояние)
        auto curr_t = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(curr_t - start_time).count();

        if (current_best == 0) {
            return {current_best, elapsed, "нулевое значение фитнесс-функции", gen};
        }
        if (elapsed > max_time_sec) {
            return {
                current_best, elapsed,
                "превышение времени работы алгоритма полного перебора (для этого же экземпляра задачи) в 2 раза и более",
                gen
            };
        }

        // Формирование нового поколения
        vector<uint32_t> next_pop(POP_SIZE);

        // Элитизм: лучший индивид копируется
        int elite_idx = 0;
        for (int i = 1; i < POP_SIZE; ++i) {
            if (fitness[i] < fitness[elite_idx]) elite_idx = i;
        }
        next_pop[0] = population[elite_idx];

        // Отбор, кроссовер, мутация
        auto tournament_select = [&]() -> uint32_t {
            int idx = uniform_int_distribution<int>(0, POP_SIZE - 1)(rng);
            int64_t best_f = fitness[idx];
            uint32_t best_c = population[idx];
            for (int t = 1; t < TOURNAMENT_SIZE; ++t) {
                int cidx = uniform_int_distribution<int>(0, POP_SIZE - 1)(rng);
                if (fitness[cidx] < best_f) {
                    best_f = fitness[cidx];
                    best_c = population[cidx];
                }
            }
            return best_c;
        };

        for (int i = 1; i < POP_SIZE; ++i) {
            uint32_t p1 = tournament_select();
            uint32_t p2 = tournament_select();

            uint32_t child = p1;
            if (uniform_real_distribution<double>(0.0, 1.0)(rng) < CROSSOVER_RATE) {
                int point = uniform_int_distribution<int>(1, n - 1)(rng);
                uint32_t low_mask = (1u << point) - 1u;
                child = (p1 & low_mask) | (p2 & ~low_mask);
            }

            // Мутация
            for (int j = 0; j < n; ++j) {
                if (uniform_real_distribution<double>(0.0, 1.0)(rng) < MUT_RATE) {
                    child ^= (1u << j);
                }
            }
            next_pop[i] = child;
        }

        population = std::move(next_pop);

        // Пересчёт фитнеса нового поколения
        int64_t new_best = evaluate(population, fitness);
        gen++;

        // Обновление stagnation
        if (new_best < current_best) {
            stagnation_count = 0;
        } else {
            stagnation_count++;
        }
        current_best = new_best;

        if (stagnation_count >= 2) {
            auto curr_t2 = chrono::high_resolution_clock::now();
            double elap = chrono::duration<double>(curr_t2 - start_time).count();
            return {
                current_best, elap, "отсутствие улучшения значения фитнесс-функции на последних двух итерациях", gen
            };
        }
    }
}

vector<int> generate_knapsack_vector(int n, int amax, mt19937 &rng) {
    uniform_int_distribution<int> dist(1, amax);
    vector<int> w(n);
    for (int &x: w) x = dist(rng);
    return w;
}

int main(int argc, char *argv[]) {
    omp_set_num_threads(8);
    const int N = 24;
    double factor = 1.4;
    int amax = pow(2.0, 24 / factor);

    cout << "Вариант: 4" << " | n = " << N << " | amax = " << amax << endl;
    cout << "OpenMP threads: " << omp_get_max_threads() << endl << endl;

    unsigned seed = 42;
    mt19937 rng(seed);

    const int NUM_VECTORS = 5;
    const int NUM_PROBS_PER_VEC = 15;

    vector<vector<int> > knapsacks(NUM_VECTORS);
    for (int i = 0; i < NUM_VECTORS; ++i) {
        knapsacks[i] = generate_knapsack_vector(N, amax, rng);
    }

    vector<Problem> problems;
    uniform_real_distribution<double> frac_dist(0.1, 0.5);
    for (int v = 0; v < NUM_VECTORS; ++v) {
        for (int p = 0; p < NUM_PROBS_PER_VEC; ++p) {
            double frac = frac_dist(rng);
            int k = max(1, static_cast<int>(round(frac * N)));
            vector<int> indices(N);
            iota(indices.begin(), indices.end(), 0);
            shuffle(indices.begin(), indices.end(), rng);
            int64_t targ = 0;
            for (int i = 0; i < k; ++i) {
                targ += knapsacks[v][indices[i]];
            }
            problems.push_back({v, targ, frac});
        }
    }

    int total_problems = problems.size();
    cout << "Сгенерировано векторов: " << NUM_VECTORS << endl;
    cout << "Сгенерировано задач: " << total_problems << endl << endl;

    vector<BruteRes> brute_results(total_problems);
    vector<GARes> ga_results(total_problems);

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < total_problems; ++i) {
        int thread_id = omp_get_thread_num();
        mt19937 local_rng(seed + static_cast<unsigned long long>(i) * 123456789ull + thread_id * 987654321ull);

        const Problem &pr = problems[i];
        const vector<int> &w = knapsacks[pr.vec_idx];

        brute_results[i] = run_brute(w, pr.target, N);

        double max_ga_time = 2.0 * brute_results[i].t_all;
        auto [fit, tga, reason, gens] = run_ga(w, pr.target, max_ga_time, local_rng, N);
        ga_results[i] = {tga, fit, reason, gens};
    }

    // === Статистическая обработка (Таблица 5) ===
    double sum_tfirst = 0.0, sum_tfirst_sq = 0.0;
    double sum_tall = 0.0, sum_tall_sq = 0.0;
    double sum_tga_exact = 0.0, sum_tga_exact_sq = 0.0;
    int exact_count = 0;

    for (int i = 0; i < total_problems; ++i) {
        double tf = brute_results[i].t_first;
        sum_tfirst += tf;
        sum_tfirst_sq += tf * tf;

        double ta = brute_results[i].t_all;
        sum_tall += ta;
        sum_tall_sq += ta * ta;

        if (ga_results[i].min_fit == 0) {
            double tg = ga_results[i].t_ga;
            sum_tga_exact += tg;
            sum_tga_exact_sq += tg * tg;
            ++exact_count;
        }
    }

    double mean_tfirst = sum_tfirst / total_problems;
    double var_tfirst = (sum_tfirst_sq / total_problems) - mean_tfirst * mean_tfirst;
    double std_tfirst = sqrt(var_tfirst);

    double mean_tall = sum_tall / total_problems;
    double var_tall = (sum_tall_sq / total_problems) - mean_tall * mean_tall;
    double std_tall = sqrt(var_tall);

    double mean_tga = (exact_count > 0 ? sum_tga_exact / exact_count : 0.0);
    double var_tga = (exact_count > 1 ? (sum_tga_exact_sq / exact_count) - mean_tga * mean_tga : 0.0);
    double std_tga = (exact_count > 0 ? sqrt(var_tga) : 0.0);

    double success_rate = static_cast<double>(exact_count) / total_problems;

    cout << fixed << setprecision(6);
    cout << "=== Таблица 5. Статистическая обработка результатов ===" << endl;
    cout << "n                              : " << N << endl;
    cout << "amax                           : " << amax << endl;
    cout << "Количество хромосом в поколении: 100" << endl << endl;

    cout << "Время нахождения одного решения полным перебором:" << endl;
    cout << "   Среднее значение            : " << mean_tfirst << " с" << endl;
    cout << "   Дисперсия                   : " << var_tfirst << endl;
    cout << "   Среднее квадратичное откл.  : " << std_tfirst << " с" << endl << endl;

    cout << "Время нахождения всех решений полным перебором:" << endl;
    cout << "   Среднее значение            : " << mean_tall << " с" << endl;
    cout << "   Дисперсия                   : " << var_tall << endl;
    cout << "   Среднее квадратичное откл.  : " << std_tall << " с" << endl << endl;

    if (exact_count > 0) {
        cout << "Время нахождения точного решения генетическим алгоритмом:" << endl;
        cout << "   Среднее значение            : " << mean_tga << " с" << endl;
        cout << "   Дисперсия                   : " << var_tga << endl;
        cout << "   Среднее квадратичное откл.  : " << std_tga << " с" << endl;
    } else {
        cout << "Время нахождения точного решения генетическим алгоритмом: нет точно решённых задач" << endl;
    }
    cout << endl;

    cout << "Доля задач, точно решённых генетическим алгоритмом: " << success_rate << " (" << exact_count << "/" <<
            total_problems << ")" << endl << endl;

    cout << "Готово! Данные для графиков (зависимость от amax) можно собирать, запуская программу для разных вариантов."
            << endl;
    cout <<
            "Для заполнения Таблиц 1–4 используйте сгенерированные данные (в коде их можно вывести в CSV при необходимости)."
            << endl;

    return 0;
}
