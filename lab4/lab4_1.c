#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define DEFAULT_ARR_SIZE 10

int main(int argc, char **argv)
{
    int rank, size;
    int local_min = INT_MAX;
    int global_min = INT_MAX;
    int array_size = 20;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        if (argc > 1)
        {
            array_size = atoi(argv[1]);
            if (array_size <= 0)
            {
                printf("Размер массива не задан, используется размер по умолчанию (20).\n");
                array_size = DEFAULT_ARR_SIZE;
            }
        }
        printf("Используемый размер массива: %d\n", array_size);
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int *array = NULL;
    if (rank == 0)
    {
        array = (int *)malloc(array_size * sizeof(int));
        srand(time(NULL));
        printf("Массив: ");
        for (int i = 0; i < array_size; ++i)
        {
            array[i] = rand() % 100 - 50;
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    int local_size = array_size / size;
    int *local_array = (int *)malloc(local_size * sizeof(int));

    MPI_Scatter(array, local_size, MPI_INT, local_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_size; ++i)
    {
        if (local_array[i] > 0 && local_array[i] < local_min)
        {
            local_min = local_array[i];
        }
    }

    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        if (global_min != INT_MAX)
        {
            printf("Минимальный положительный элемент: %d\n", global_min);
        }
        else
        {
            printf("В массиве нет положительных элементов.\n");
        }
        free(array);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}