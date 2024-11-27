#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>
#include <stdbool.h>

#define RUNS_NUM 1
#define NUMBER (int) 1e3

void save_res(const double time){
    FILE *file = fopen("/Users/maxim/Desktop/All/code/CLionProjects/ParProc/lab7/res.txt", "a");

    if (!file){
        perror("Error opening file");
    }

    fprintf(file, "%lf\n", time);
    fclose(file);
}

int *findPrimesInRangePre(int S, int N, int *primeCount){
    if (N < 2 || S > N){
        *primeCount = 0;
        return NULL;
    }

    bool *isPrime = (bool *) malloc((N - S + 1) * sizeof(bool));

    isPrime[0] = isPrime[1] = false;

    for (int i = 2; i <= N; ++i){
        isPrime[i] = true;
    }

    for (int i = 2; i <= sqrt(N); ++i){
        if (isPrime[i]){
            for (int j = i * i; j <= N; j += i){
                isPrime[j] = false;
            }
        }
    }

    int *primes = (int *) malloc((N - S + 1) * sizeof(int));
    *primeCount = 0;

    for (int i = (S < 2 ? 2 : S); i <= N; ++i){
        if (isPrime[i]){
            primes[(*primeCount)++] = i;
        }
    }
    primes = realloc(primes, *primeCount * sizeof(int));

    free(isPrime);

    return primes;
}

int *findPrimesInRange(int S, int N, const int *divisors, int divisorCount, int *primeCount){
    if (N < 2 || S > N){
        *primeCount = 0;
        return NULL;
    }

    bool *isPrime = (bool *) malloc((N - S + 1) * sizeof(bool));

    isPrime[0] = isPrime[1] = false;

    for (int i = 2; i < N - S + 1; ++i){
        isPrime[i] = true;
    }

//#pragma omp parallel for num_threads(2) default(shared)
    for (int i = 0; i < divisorCount; ++i){
        int p = divisors[i];
        int start = fmax(p * p, ((S + p - 1) / p) * p);

        for (int j = start; j <= N; j += p){
            isPrime[j - S] = false;
        }
    }

    int *primes = (int *) malloc((N - S + 1) * sizeof(int));
    *primeCount = 0;

#pragma omp parallel for num_threads(2) default(shared)
    for (int i = (S < 2 ? 2 : S); i <= N; ++i){
        if (isPrime[i - S]){
            primes[(*primeCount)++] = i;
        }
    }
    primes = realloc(primes, *primeCount * sizeof(int));

    free(isPrime);

    return primes;
}

void combineResults(const int S, const int N, const int *sharedPrimes, int sharedPrimesCount, const int rank,
                    const int proc_num){
    int localPrimesCount = 0;
    int *localPrimes = findPrimesInRange(S, N, sharedPrimes, sharedPrimesCount, &localPrimesCount);

    int totalCount;
    int *array = NULL;
    int *recvcounts = NULL;
    int *displs = NULL;

    if (!rank){
        recvcounts = (int *) malloc(proc_num * sizeof(int));
        displs = (int *) malloc(proc_num * sizeof(int));
    }

    MPI_Gather(&localPrimesCount, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (!rank){
        totalCount = 0;
        for (int i = 0; i < proc_num; ++i){
            displs[i] = totalCount;
            totalCount += recvcounts[i];
        }

        array = (int *) malloc(totalCount * sizeof(int));
    }

    MPI_Gatherv(localPrimes, localPrimesCount, MPI_INT, array, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (!rank){
         for (int i = 0; i < totalCount; i++) {
             printf("%d ", array[i]);
         }
         printf("\n");

        free(recvcounts);
        free(displs);
        free(array);
    }

    free(localPrimes);
}

void time_algorithm(){
    double timeSpent = 0;
    double maxTimeSpent = 0;
    int proc_num = -1;
    int rank = -1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);

    int sharedPrimesCount = 0;
    int *sharedPrimes = NULL;
    int sqrtN = sqrt(NUMBER);
    int workingRange = NUMBER - sqrtN;

    if (!rank){
        sharedPrimes = findPrimesInRangePre(0, sqrtN, &sharedPrimesCount);
    }

    MPI_Bcast(&sharedPrimesCount, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

    if (rank){
        sharedPrimes = malloc(sharedPrimesCount * sizeof(int));
    }

    MPI_Bcast(sharedPrimes, sharedPrimesCount, MPI_INTEGER, 0, MPI_COMM_WORLD);

    int S = sqrtN + rank * workingRange / proc_num;
    int N = sqrtN + (rank + 1) * workingRange / proc_num;

    for (int i = 0; i < RUNS_NUM; i++){
        const double start = MPI_Wtime();
        combineResults(S, N, sharedPrimes, sharedPrimesCount, rank, proc_num);
        const double end = MPI_Wtime();
        timeSpent += end - start;
    }

    MPI_Reduce(&timeSpent, &maxTimeSpent, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (!rank){
        save_res(maxTimeSpent / RUNS_NUM);
        printf("%lf", maxTimeSpent / RUNS_NUM);
        free(sharedPrimes);
    }
}

int main(int argc, char **argv){
    int ret = -1; ///< For return values
    int rank = -1; ///< This processor's number

    ret = MPI_Init(&argc, &argv);
    if (!rank){ printf("MPI Init returned (%d)\n", ret); }

    time_algorithm();

    ret = MPI_Finalize();

    return 0;
}
