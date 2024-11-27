//#include <cstdlib>
//#include <cstdio>
//#include <mpi.h>
//#include <omp.h>
//#include <algorithm>
//#include <cmath>
//#include <iostream>
//
//#define RUNS_NUM 1
//#define NUMBER static_cast<int>(1e5)
//
//void save_res(const double &time) {
//    FILE *file = fopen("/Users/maxim/Desktop/All/code/CLionProjects/ParProc/lab7/res.txt", "a");
//
//    if (!file) {
//        perror("Error opening file.");
//    }
//
//    fprintf(file, "%lf\n", time);
//    fclose(file);
//}
//
//int *findPrimesInRange(const int &S, const int &N, int &primeCount) {
//    if (N < 2 || S > N) {
//        primeCount = 0;
//        return nullptr;
//    }
//
//    bool *isPrime = new bool[N + 1];
//
//    isPrime[0] = isPrime[1] = false;
//    std::fill(isPrime + 2, isPrime + N + 1, true);
//
//    for (int i = 2; i <= std::sqrt(N); ++i) {
//        if (isPrime[i]) {
//            for (int j = i * i; j <= N; j += i) {
//                isPrime[j] = false;
//            }
//        }
//    }
//
//    int *primes = new int[N - S + 1];
//    primeCount = 0;
//
//    for (int i = std::max(S, 2); i <= N; ++i) {
//        if (isPrime[i]) {
//            primes[primeCount++] = i;
//        }
//    }
//
//    int *resizedPrimes = new int[primeCount];
//    std::copy(primes, primes + primeCount, resizedPrimes);
//
//    delete[] primes;
//    delete[] isPrime;
//    return resizedPrimes;
//}
//
//int *findPrimesInRange(const int &S, const int &N, const int *divisors, const int &divisorCount, int &primeCount) {
//    bool *isPrime = new bool[N - S + 1];
//    std::fill(isPrime, isPrime + (N - S + 1), true);
//
//#pragma omp parallel for num_threads(2)
//    for (int i = 0; i < divisorCount; ++i) {
//        int p = divisors[i];
//        int start = std::max(p * p, (S + p - 1) / p * p);
//
//        for (int j = start; j <= N; j += p) {
//            isPrime[j - S] = false;
//        }
//    }
//
//    int *primes = new int[N - S + 1];
//    primeCount = 0;
//
//#pragma omp parallel for num_threads(2)
//    for (int i = std::max(S, 2); i <= N; ++i) {
//        if (isPrime[i - S]) {
//            primes[primeCount++] = i;
//        }
//    }
//
//    int *resizedPrimes = new int[primeCount];
//    std::copy(primes, primes + primeCount, resizedPrimes);
//
//    delete[] primes;
//    delete[] isPrime;
//    return resizedPrimes;
//}
//
//void combineResults(const int &S, const int &N, const int *sharedPrimes, int &sharedPrimesCount, const int &rank,
//                    const int &proc_num) {
//    int localPrimesCount = 0;
//    int *localPrimes = findPrimesInRange(S, N, sharedPrimes, sharedPrimesCount, localPrimesCount);
//
//    int totalCount;
//    int *array = nullptr;
//    int *recvcounts = nullptr;
//    int *displs = nullptr;
//
//    if (!rank) {
//        recvcounts = new int[proc_num];
//        displs = new int[proc_num];
//    }
//
//    MPI_Gather(&localPrimesCount, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
//
//    if (!rank) {
//        totalCount = 0;
//
//        for (int i = 0; i < proc_num; ++i) {
//            displs[i] = totalCount;
//            totalCount += recvcounts[i];
//        }
//
//        array = new int[totalCount];
//    }
//
//    MPI_Gatherv(localPrimes, localPrimesCount, MPI_INT, array, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
//
//    if (!rank) {
//        delete[] recvcounts;
//        delete[] displs;
//
////        for (int i = 0; i < totalCount; i++){
////            printf("%d ", array[i]);
////        }
////        printf("\n");
//    }
//
//    delete[] localPrimes;
//}
//
//void time_algorithm() {
//    double timeSpent = 0;
//    double maxTimeSpent = 0;
//    int proc_num = -1;
//    int rank = -1;
//
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
//
//    int sharedPrimesCount = 0;
//    int *sharedPrimes = nullptr;
//    int sqrtN = std::sqrt(NUMBER);
//    int workingRange = NUMBER - sqrtN;
//
//    if (!rank) {
//        sharedPrimes = findPrimesInRange(0, sqrtN, sharedPrimesCount);
//    }
//
//    MPI_Bcast(&sharedPrimesCount, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
//    MPI_Bcast(sharedPrimes, sharedPrimesCount, MPI_INTEGER, 0, MPI_COMM_WORLD);
//
//    int S = sqrtN + rank * workingRange / proc_num;
//    int N = sqrtN + (rank + 1) * workingRange / proc_num;
//
//    for (int i = 0; i < RUNS_NUM; i++) {
//        const double start = MPI_Wtime();
//        combineResults(S, N, sharedPrimes, sharedPrimesCount, rank, proc_num);
//        const double end = MPI_Wtime();
//        timeSpent += end - start;
//    }
//
//    MPI_Reduce(&timeSpent, &maxTimeSpent, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
//
//    if (!rank) {
//        save_res(maxTimeSpent / RUNS_NUM);
//        printf("%lf", maxTimeSpent / RUNS_NUM);
//    }
//}
//
//
//int main(int argc, char **argv) {
//    int ret = -1; ///< For return values
//    int rank = -1; ///< This processor's number
//
//    ret = MPI_Init(&argc, &argv);
//    if (!rank) { printf("MPI Init returned (%d)\n", ret); }
//
//    time_algorithm();
//
//    ret = MPI_Finalize();
//
//    return 0;
//}
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cout << "Hello from processor " << rank << "\n";
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}