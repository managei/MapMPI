// to compile: mpicc PDC_Project.c -lm
// to run: mpiexec -n 8 ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

void calculateNumberOfMappersAndReducers(long int sizeOfMatrix, int numberOfSlaves, int *mappers, int *reducers, long int *inputSplit, bool *equalDivisionPossible)
{
    long long int totalElementsInMatrix = sizeOfMatrix * sizeOfMatrix;
    int numberOfMappers = numberOfSlaves - 1;          // at least one slave is required for reducer
    int mod = totalElementsInMatrix % numberOfMappers; // see if equal division is possible

    while (mod != 0 && numberOfMappers > ((numberOfSlaves / 2) + 1))
    {
        numberOfMappers--;
        mod = totalElementsInMatrix % numberOfMappers;
        printf("mod: %d\n", mod);
        printf("numberOfMappers: %d\n", numberOfMappers);
    }

    *mappers = numberOfMappers;
    *reducers = numberOfSlaves - numberOfMappers;
    *inputSplit = totalElementsInMatrix / numberOfMappers;
    mod = totalElementsInMatrix % numberOfMappers;

    if (mod == 0)
        *equalDivisionPossible = true;
    else
        *equalDivisionPossible = false;

    return;
}

void main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    srand(time(0));
    int rank = -1;
    int numberOfProcesses = 0;
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int lengthOfProcessorName = MPI_MAX_PROCESSOR_NAME;

    long int sizeOfMatrix = pow(2, rand() % 20 + 12); // assumed size of matrix

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Get_processor_name(processorName, &lengthOfProcessorName);

    if (rank == 0)
    {
        int mappers = 0;
        int reducers = 0;
        long int inputSplit = 0;
        bool equalDivisionPossible = false;
        calculateNumberOfMappersAndReducers(sizeOfMatrix, numberOfProcesses - 1, &mappers, &reducers, &inputSplit, &equalDivisionPossible);
        printf("Master with running id %d running on %s\n", rank, processorName);
        printf("Size of matrix: %ld\n", sizeOfMatrix);
        printf("Number of mappers: %d\n", mappers);
        printf("Number of reducers: %d\n", reducers);
        printf("Input split: %ld\n", inputSplit);
        printf("Equal division possible: %s\n", equalDivisionPossible ? "true" : "false");
    }
    else
    {
        // printf("This is slave process %d\n", rank);
    }

    MPI_Finalize();
}