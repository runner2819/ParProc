#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

bool is_prime(int num) {
    if (num < 2) return false;
    int limit = static_cast<int>(std::sqrt(num));
    for (int i = 2; i <= limit; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int Nstart, Nend;
    if (rank == 0) {
        std::cout << "Enter the range (Nstart Nend): \n";
        std::cin >> Nstart >> Nend;
    }

    // Рассылаем границы диапазона всем процессам
    MPI_Bcast(&Nstart, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Nend, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int range = (Nend - Nstart + 1) / size;
    int local_start = Nstart + rank * range;
    int local_end = (rank == size - 1) ? Nend : local_start + range - 1;

    // Установка количества запусков
    const int num_runs = 15;
    double total_time = 0.0;

    for (int run = 0; run < num_runs; ++run) {
        double start_time = MPI_Wtime(); // Старт таймера

        // Поиск простых чисел в локальном диапазоне
        std::vector<int> local_primes;
        #pragma omp parallel
        {
            std::vector<int> local_primes_private;
            #pragma omp for schedule(dynamic)
            for (int i = local_start; i <= local_end; ++i) {
                if (is_prime(i)) {
                    local_primes_private.push_back(i);
                }
            }
            #pragma omp critical
            local_primes.insert(local_primes.end(), local_primes_private.begin(), local_primes_private.end());
        }

        // Сбор всех найденных простых чисел на корневом процессе
        if (rank == 0) {
            std::vector<int> all_primes;
            for (int i = 0; i < size; ++i) {
                if (i == 0) {
                    all_primes = local_primes;
                } else {
                    int recv_count;
                    MPI_Recv(&recv_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    std::vector<int> recv_primes(recv_count);
                    MPI_Recv(recv_primes.data(), recv_count, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    all_primes.insert(all_primes.end(), recv_primes.begin(), recv_primes.end());
                }
            }

            // Сортировка результатов (только для первого запуска)
            if (run == 0) {
                std::sort(all_primes.begin(), all_primes.end());
//                std::cout << "Primes in range [" << Nstart << ", " << Nend << "]:" << std::endl;
//                for (int prime : all_primes) {
//                    std::cout << prime << " ";
//                }
//                std::cout << std::endl;
            }
        } else {
            int local_count = local_primes.size();
            MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(local_primes.data(), local_count, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }

        double end_time = MPI_Wtime(); // Конец таймера
        total_time += (end_time - start_time); // Суммируем время выполнения
    }

    // Усреднение времени выполнения
    if (rank == 0) {
        double average_time = total_time / num_runs;
        std::cout << "Average execution time over " << num_runs << " runs: " << average_time << " seconds." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
