// to compile: mpicc PDC_Project.c -lm
// to run: mpiexec -n 8 ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

// ftn to calculate number of mappers and reducers
// returns sizeOfMatrix, mappers, reducers, inputSplit, equalDivisionPossible, remainingSplit
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

// return whether the rank is of a mapper: bool
bool isMapper(int rank, int mappers)
{
    return rank > 0 && rank <= mappers;
}

// return whether the rank is of a reducer: bool
bool isReducer(int rank, int mappers)
{
    return rank > mappers;
}

// return whether the rank is of a master: bool
bool isMaster(int rank)
{
    return rank == 0;
}

void main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv); // initialize MPI
    srand(time(0));
    int rank = -1;
    int numberOfProcesses = 0;
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int lengthOfProcessorName = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);              // world rank
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses); // world size
    MPI_Get_processor_name(processorName, &lengthOfProcessorName);

    long long int sizeOfMatrix = 0;
    int mappers = 0;  // number of mappers
    int reducers = 0; // number of reducers
    int equalDivisionPossible = 0;

    if (isMaster(rank)) // master node
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
    MPI_Bcast(&sizeOfMatrix, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD); // broadcast size of matrix
    MPI_Bcast(&mappers, 1, MPI_INT, 0, MPI_COMM_WORLD);                // broadcast number of mappers

    reducers = (numberOfProcesses - 1) - mappers; // calculate number of reducers = numberOfSlaves - numberOfMappers

    MPI_Group MPI_GROUP_WORLD;
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD); // fetch group from MPI_COMM_WORLD

    if (isMaster(rank))
    {
        // task distribution here

        MPI_Barrier(MPI_COMM_WORLD); // task distribution completed

        // send data to mappers here use blocking send

        MPI_Barrier(MPI_COMM_WORLD); // Mappers completed processing

        // shuffle here and see what to do for synchronization

        // send data to reducers here use blocking send

        MPI_Barrier(MPI_COMM_WORLD); // Reducers completed processing
    }
    else if (isMapper(rank, mappers)) // mapper node code
    {
        MPI_Barrier(MPI_COMM_WORLD); // wait for task distribution
        MPI_Comm MPI_COMM_MAPPER;    // mapper communicator
        MPI_Group MPI_GROUP_MAPPER;
        int mapperRankInMapperComm = 0; // rank of mapper in mapper communicator
        int mapperRanks[mappers];

        int curMapperRank = 1; // compute mapper ranks for mapper communicator
        for (int i = 0; i < mappers; i++)
        {
            mapperRanks[i] = curMapperRank++;
        }

        MPI_Group_incl(MPI_GROUP_WORLD, mappers, mapperRanks, &MPI_GROUP_MAPPER);     // create group of mappers
        MPI_Comm_create_group(MPI_COMM_WORLD, MPI_GROUP_MAPPER, 1, &MPI_COMM_MAPPER); // create mapper communicator
        MPI_Comm_rank(MPI_COMM_MAPPER, &mapperRankInMapperComm);                      // get rank of current mapper in mapper communicator

        printf("Mapper with running id %d running on %s and mapper rank %d\n", rank, processorName, mapperRankInMapperComm);

        // receive data from master here use blocking recieve

        // loop for processing here

        MPI_Barrier(MPI_COMM_WORLD); // all mappers completed processing it is ok to shuffle
        MPI_Barrier(MPI_COMM_WORLD); // allow reducers to complete processing
    }
    else if (isReducer(rank, mappers)) // reducer node code
    {
        MPI_Barrier(MPI_COMM_WORLD); // wait for task distribution
        MPI_Barrier(MPI_COMM_WORLD); // wait for shuffling

        MPI_Comm MPI_COMM_REDUCER; // reducer communicator
        MPI_Group MPI_GROUP_REDUCER;
        int reducerRankInReducerComm = 0;
        int reducerRanks[reducers];

        int curReducerRank = mappers + 1; // compute reducer ranks for reducer communicator
        for (int i = 0; i < reducers; i++)
        {
            reducerRanks[i] = curReducerRank++;
        }

        MPI_Group_incl(MPI_GROUP_WORLD, reducers, reducerRanks, &MPI_GROUP_REDUCER);    // create group of reducers
        MPI_Comm_create_group(MPI_COMM_WORLD, MPI_GROUP_REDUCER, 2, &MPI_COMM_REDUCER); // create reducer communicator
        MPI_Comm_rank(MPI_COMM_REDUCER, &reducerRankInReducerComm);                     // get rank of current reducer in reducer communicator

        printf("Reducer with running id %d running on %s and reducer rank %d\n", rank, processorName, reducerRankInReducerComm);

        // receive data from master here use blocking recieve

        // loop for processing here

        MPI_Barrier(MPI_COMM_WORLD); // all reducers completed processing
    }

    MPI_Finalize(); // finalize MPI environment
}