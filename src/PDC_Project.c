// to compile: mpicc PDC_Project.c -lm
// to run: mpiexec -n 8 ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

void calculateNumberOfMappersAndReducers(long int sizeOfMatrix, int numberOfSlaves, int *mappers, int *reducers, long long int *inputSplit, int *equalDivisionPossible, int *remainingSplit)
{
    long long int totalElementsInMatrix = sizeOfMatrix * sizeOfMatrix;
    int numberOfMappers = numberOfSlaves - 1;          // at least one slave is required for reducer
    int mod = totalElementsInMatrix % numberOfMappers; // see if equal division is possible

    while (mod != 0 && numberOfMappers > ((numberOfSlaves / 2) + 1))
    {
        numberOfMappers--;
        mod = totalElementsInMatrix % numberOfMappers;
        // printf("mod: %d\n", mod);
        // printf("numberOfMappers: %d\n", numberOfMappers);
    }

    *mappers = numberOfMappers;
    *reducers = numberOfSlaves - numberOfMappers;
    *inputSplit = totalElementsInMatrix / numberOfMappers;
    mod = totalElementsInMatrix % numberOfMappers;

    if (mod == 0)
    {
        *equalDivisionPossible = 1;
        *remainingSplit = 0;
    }
    else
    {
        *equalDivisionPossible = 0;
        *remainingSplit = mod;
    }

    return;
}

void main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    srand(time(0));
    int rank = -1;
    int numberOfProcesses = 0;
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int lengthOfProcessorName = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Get_processor_name(processorName, &lengthOfProcessorName);

    long long int sizeOfMatrix = 0;
    int mappers = 0;
    int reducers = 0;
    int equalDivisionPossible = 0;

    if (rank == 0)
    {
        printf("Master with running id %d running on %s\n", rank, processorName);

        sizeOfMatrix = pow(2, rand() % 20 + 12); // assumed size of matrix
        long long int inputSplit = 0;
        int remainingSplit = 0;
        calculateNumberOfMappersAndReducers(sizeOfMatrix, numberOfProcesses - 1, &mappers, &reducers, &inputSplit, &equalDivisionPossible, &remainingSplit);

        printf("Size of matrix: %lld\n", sizeOfMatrix);
        printf("Number of mappers: %d\n", mappers);
        printf("Number of reducers: %d\n", reducers);
        printf("Input split: %lld\n", inputSplit);
        printf("Remaining split: %d\n", remainingSplit);
        printf("Equal division possible: %s\n", equalDivisionPossible ? "true" : "false");
    }
    MPI_Bcast(&sizeOfMatrix, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mappers, 1, MPI_INT, 0, MPI_COMM_WORLD);

    reducers = (numberOfProcesses - 1) - mappers;

    MPI_Group MPI_GROUP_WORLD;
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD); // fetch group from MPI_COMM_WORLD

    if (rank > 0 && rank <= mappers)
    {
        MPI_Comm MPI_COMM_MAPPER;
        MPI_Group MPI_GROUP_MAPPER;
        int mapperRankInMapperComm = 0;
        int mapperRanks[mappers];

        int curMapperRank = 1;
        for (int i = 0; i < mappers; i++)
        {
            mapperRanks[i] = curMapperRank++;
        }
        MPI_Group_incl(MPI_GROUP_WORLD, mappers, mapperRanks, &MPI_GROUP_MAPPER); // create group of mappers
        MPI_Comm_create_group(MPI_COMM_WORLD, MPI_GROUP_MAPPER, 1, &MPI_COMM_MAPPER);
        MPI_Comm_rank(MPI_COMM_MAPPER, &mapperRankInMapperComm);

        printf("Mapper with running id %d running on %s and mapper rank %d\n", rank, processorName, mapperRankInMapperComm);
    }
    else if (rank > mappers)
    {
        MPI_Comm MPI_COMM_REDUCER;
        MPI_Group MPI_GROUP_REDUCER;
        int reducerRankInReducerComm = 0;
        int reducerRanks[reducers];

        int curReducerRank = mappers + 1;
        for (int i = 0; i < reducers; i++)
        {
            reducerRanks[i] = curReducerRank++;
        }

        MPI_Group_incl(MPI_GROUP_WORLD, reducers, reducerRanks, &MPI_GROUP_REDUCER); // create group of reducers
        MPI_Comm_create_group(MPI_COMM_WORLD, MPI_GROUP_REDUCER, 2, &MPI_COMM_REDUCER);
        MPI_Comm_rank(MPI_COMM_REDUCER, &reducerRankInReducerComm);

        printf("Reducer with running id %d running on %s and reducer rank %d\n", rank, processorName, reducerRankInReducerComm);
    }

    MPI_Finalize();
}