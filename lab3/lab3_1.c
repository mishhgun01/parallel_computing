#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROWS 6
#define COLS 4

void print_matrix(int matrix[ROWS][COLS], int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int matrix[ROWS][COLS];
    int local_rows = ROWS / (size - 1);
    int local_matrix[local_rows][COLS];

    if (rank == 0)
    {
        printf("Исходная матрица:\n");
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                matrix[i][j] = rand() % 2;
            }
        }
        print_matrix(matrix, ROWS, COLS);
    }

    MPI_Scatter(matrix, local_rows * COLS, MPI_INT, local_matrix, local_rows * COLS, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_rows; i++)
    {
        for (int j = 1; j < COLS; j += 2)
        {
            local_matrix[i][j] = 1 - local_matrix[i][j];
        }
    }

    MPI_Gather(local_matrix, local_rows * COLS, MPI_INT, matrix, local_rows * COLS, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Матрица после инверсии четных столбцов:\n");
        print_matrix(matrix, ROWS, COLS);
    }

    MPI_Finalize();
    return 0;
}