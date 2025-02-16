#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv)
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (int i = 1; i <= 10; ++i)
    {
        printf("%d*%d=%d\n", rank + 1, i, (rank + 1) * i);
    }
    MPI_Finalize();

    return 0;
}
