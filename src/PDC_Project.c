// to compile: mpicc PDC_Project.c
// to run: mpiexec -n 8 ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank = -1;
    int numberOfProcesses = 0;
    char processorName[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    if (rank == 0)
    {
        printf("This is master process\n");
    }
    else
    {
        printf("This is slave process %d\n", rank);
    }

    MPI_Finalize();
}