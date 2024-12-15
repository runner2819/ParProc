#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5000000
double tiempo_inicio;
double tiempo_termino;

int comp(const void *a, const void *b) {
    return (*(int *) a - *(int *) b);
}

void Imprimir(int *array, int length) //print array elements
{
    for (int i = 0; i < length; i++)
        printf("%d\n", array[i]);
}

int *merge(int *arr1, int n1, int *arr2, int n2) {
    int *result = malloc((n1 + n2) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < n1 && j < n2)
        if (arr1[i] < arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    while (i < n1) {
        result[k++] = arr1[i++];
    }
    while (j < n2) {
        result[k++] = arr2[j++];
    }

    return result;
}

void shellsort(int numbers[], int array_size) {
    int i, j, increment, temp;
    increment = 3;
    while (increment > 0) {
        for (i = 0; i < array_size; i++) {
            j = i;
            temp = numbers[i];
            while ((j >= increment) && (numbers[j - increment] > temp)) {
                numbers[j] = numbers[j - increment];
                j = j - increment;
            }
            numbers[j] = temp;
        }
        if (increment / 2 != 0)
            increment = increment / 2;
        else if (increment == 1)
            increment = 0;
        else
            increment = 1;
    }
}

int main(int argc, char **argv) {
    int *data;
//    int *ddata;
    int *chunk;
    int *other;
    int m, n = N;
    int rank, size;
    int s;
    int i;
    int step;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int r;
        s = n / size;
        r = n % size;
        data = (int *) malloc((n + size - r) * sizeof(int));
//        ddata = (int *) malloc((n + p - r) * sizeof(int));
        printf("llenando vector (%d datos)...\n", N);
        for (i = 0; i < n; i++)
            data[i] = N - i;
//        memcpy(ddata, data, n * sizeof(int));

        if (r != 0) {
            for (i = n; i < n + size - r; i++)
                data[i] = 0;
            s = s + 1;
        }
        printf("Ejecutando shellsort paralelo...\n");
        tiempo_inicio = MPI_Wtime();

        MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
        chunk = (int *) malloc(s * sizeof(int));
        MPI_Scatter(data, s, MPI_INT, chunk, s, MPI_INT, 0, MPI_COMM_WORLD);
        shellsort(chunk, s);
    }


/**************************** worker task ************************************/
    else {
        MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
        chunk = (int *) malloc(s * sizeof(int));
        MPI_Scatter(data, s, MPI_INT, chunk, s, MPI_INT, 0, MPI_COMM_WORLD);
        shellsort(chunk, s);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        printf("barrie\n");
    }
    sleep(30);

    step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                MPI_Recv(&m, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);
                other = (int *) malloc(m * sizeof(int));
                MPI_Recv(other, m, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);
                int *res = merge(chunk, s, other, m);

                free(other);
                free(chunk);

                chunk = res;
                s = s + m;
//                printf("%d,%d\n", id, id + step);

            }
        } else {
            int near = rank - step;
            MPI_Send(&s, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
            MPI_Send(chunk, s, MPI_INT, near, 0, MPI_COMM_WORLD);
            free(chunk);
            break;
        }
        step = step * 2;
    }
    if (rank == 0) {
        printf("barrie\n");
    }
    sleep(30);
    if (rank == 0) {
//        qsort(ddata, n, sizeof(int), comp);
//        printf("\nres%d\n", memcmp(ddata, chunk, n * sizeof(int)));
        tiempo_termino = MPI_Wtime();
        printf("Tiempo de ejecucion = %f segundos\n", (tiempo_termino - tiempo_inicio) / CLOCKS_PER_SEC);
        //Imprimir(chunk,N);
    }

    MPI_Finalize();

}