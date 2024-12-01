std::vector<bool> create_mask(const std::vector<int>& bases) {
    int period = 1;
    for (int base : bases) {
        period *= base;
    }

    std::vector<bool> mask(period, true);
    for (int base : bases) {
        for (int i = base; i <= period; i += base) {
            mask[i - 1] = false;
        }
    }
    return mask;
}



bool is_prime(int num, std::vector<bool> mask, int period) {
    if (num < 2) return false;
    int limit = static_cast<int>(std::sqrt(num));
    
    if (!mask[((num - 1) % period)]){
        return false;
    } 
    //if (num % 2 == 0 && num != 2) return false;

    for (int i = 2; i <= limit; i++) {
        
        if (num % i == 0)
        { 
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    omp_set_num_threads(2);

    int Nstart = 2, Nend = 100000000;
    
    if (rank == 0) {
        std::cout << "Enter the range (Nstart Nend): ";
        std::cin >> Nstart >> Nend;
        //auto mask = create_mask({2,3});
    }

    auto mask = create_mask({2,3,5,7,11});

    // Рассылаем границы диапазона всем процессам
    MPI_Bcast(&Nstart, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Nend, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int range = (Nend - Nstart + 1) / size;
    int local_start = Nstart + rank * range;
    int local_end = (rank == size - 1) ? Nend : local_start + range - 1;
    int position;

    // Установка количества запусков
    const int num_runs = 1;
    double total_time = 0.0, all_time = 0.0;

    for (int run = 0; run < num_runs; ++run) {
        double start_time = MPI_Wtime(); // Старт таймера

        // Поиск простых чисел в локальном диапазоне
        std::vector<int> local_primes;

        int period = mask.size();

        #pragma omp parallel private(position)
        {
            std::vector<int> local_primes_private;
            #pragma omp for schedule(dynamic)
            for (int i = local_start; i <= local_end; i++) {
                
                if (is_prime(i, mask, period)){
                    local_primes_private.push_back(i);
                }
            }
            #pragma omp critical
            local_primes.insert(local_primes.end(), local_primes_private.begin(), local_primes_private.end());