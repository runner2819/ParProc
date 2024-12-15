#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv) {
    int size, rank;
    const int bigsize = 50;
    const int stride = 5;
    int tmp = bigsize % stride;
    const int count = bigsize / stride;

    const int receiver = 1;
    const int mytag = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm comm;

    int gsize,recvarray[100][150],*rptr;
    int root, *sendbuf, myrank, bufsize, *stride;
    MPI_Datatype rtype;
    int i, *displs, *scounts, offset;
    ...
    MPI_Comm_size( comm, &gsize);
    MPI_Comm_rank( comm, &myrank );

    stride = (int *)malloc(gsize*sizeof(int));
    ...
    /* stride[i] for i = 0 to gsize-1 is set somehow
     * sendbuf comes from elsewhere
     */
    ...
    displs = (int *)malloc(gsize*sizeof(int));
    scounts = (int *)malloc(gsize*sizeof(int));
    offset = 0;
    for (i=0; i<gsize; ++i) {
        displs[i] = offset;
        offset += stride[i];
        scounts[i] = 100 - i;
    }
    /* Create datatype for the column we are receiving
     */
    MPI_Type_vector( 100-myrank, 1, 150, MPI_INT, &rtype);
    MPI_Type_commit( &rtype );
    rptr = &recvarray[0][myrank];
    MPI_Scatterv(sendbuf, scounts, displs, MPI_INT,rptr, 1, rtype, root, comm);
    if (size < 2) {
        fprintf(stderr, "%s: Require at least two processors.\n", argv[0]);
        MPI_Finalize();
        exit(-1);
    }

    const long wstart = (long) (rank) * stride / size;
    const long wend = (long) (rank + 1) * stride / size;

    if (!rank) {
        int bigarray[bigsize];
        for (int i = 0; i < bigsize; i++) {
            bigarray[i] = i;
        }

        printf("[%d]: ", rank);
        for (int i = 0; i < bigsize; i++) {
            printf("%d ", bigarray[i]);
        }
        printf("\n");

        MPI_Datatype everyfifth;
        MPI_Type_vector(count, 1, stride, MPI_INT, &everyfifth);
        MPI_Type_commit(&everyfifth);
        int *scounts = malloc(size * sizeof(int));
        int *dislp = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            scounts[i] = count;
            if (i <= tmp) scounts[i]++;
        }
        MPI_Scatterv(bigarray,scounts,)
        MPI_Send(bigarray, 1, everyfifth, receiver, mytag, MPI_COMM_WORLD);

        MPI_Type_free(&everyfifth);
    } else if (rank == receiver) {
        int littlearray[count];

        MPI_Status status;

        MPI_Recv(littlearray, count, MPI_INT, 0, mytag,
                 MPI_COMM_WORLD, &status);

        printf("[%d]: ", rank);
        for (int i = 0; i < count; i++)
            printf("%d ", littlearray[i]);
        printf("\n");
    }

    MPI_Finalize();

    return 0;
}