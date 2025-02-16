#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int rank, size;
    int ai, bi, ci;
    int received_a, received_b;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4)
    {
        if (rank == 0)
        {
            printf("Эта программа требует ровно 4 процесса.\n");
        }
        MPI_Finalize();
        return 1;
    }

    ai = rank + 1;
    bi = (rank + 1) * 10;
    ci = 0;
    printf("Процесс %d: ai = %d, bi = %d, ci = %d\n", rank, ai, bi, ci);

    if (rank == 0)
    {
        MPI_Send(&ai, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&bi, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
        MPI_Recv(&received_a, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&received_b, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (rank == 1)
    {
        MPI_Send(&ai, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&bi, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        MPI_Recv(&received_a, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&received_b, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (rank == 2)
    {
        MPI_Send(&ai, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(&bi, 1, MPI_INT, 3, 1, MPI_COMM_WORLD);
        MPI_Recv(&received_a, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&received_b, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (rank == 3)
    {
        MPI_Send(&ai, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&bi, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Recv(&received_a, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&received_b, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("Процесс %d получил данные: a = %d, b = %d\n", rank, received_a, received_b);

    if (rank == 0)
    {
        ci = received_a + received_b;
    }
    else if (rank == 1)
    {
        ci = received_a + received_b;
    }
    else if (rank == 2)
    {
        ci = received_a + received_b;
    }
    else if (rank == 3)
    {
        ci = received_a + received_b;
    }

    printf("Процесс %d: ci = %d\n", rank, ci);
    MPI_Finalize();
    return 0;
}