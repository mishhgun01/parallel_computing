#include <stdio.h>
#include "mpi.h"
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("номер процесса - %i, PID - %i \n", rank, getpid());

	MPI_Finalize();

	return 0;
}