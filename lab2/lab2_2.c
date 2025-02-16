#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    int rank, size;
    int min_positive = INT_MAX;

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank * time(NULL));

    int random_number = rand() % 100 + 1;
    printf("Процесс %d сгенерировал число: %d\n", rank, random_number);

    if (random_number > 0 && random_number < min_positive)
    {
        min_positive = random_number;
    }

    // Собираем минимальные значения со всех процессов
    int global_min_positive;
    MPI_Reduce(&min_positive, &global_min_positive, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Минимальный положительный элемент: %d\n", global_min_positive);
    }

    MPI_Finalize();
    return 0;
}