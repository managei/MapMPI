// to compile: 
// mpicc PDC_Project.c -lm
// to run: MAKE SURE TO CREATE 3 SIZE MATRIX FIRST
// mpiexec -n 8 ./a.out 3
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

typedef struct
{   
    char name;
    int i;
    int j;
    int value;
} matrixIndex;

// This method changes the custom format matrix to 2d-matrix
void read_matrix_indexWise(char* filename1, char* filename2, matrixIndex *matrix, int totalIndexes);

// This method print matrix in index wise format 
void printMatrixIndexWise(matrixIndex *matrix, int totalIndexes);

// This method send data from master to mappers
void send_data_to_mappers(matrixIndex *inputMatrixes,int totalIndexes, long long int  inputSplit, int remainingSplit,int mappers);

// This method recieve data from master to mappers
void recieve_data_from_master(matrixIndex *inputMatrixes,int* totalIndexes);

// ftn to calculate number of mappers and reducers
// returns sizeOfMatrix, mappers, reducers, inputSplit, equalDivisionPossible, remainingSplit
void calculateNumberOfMappersAndReducers(long long int totalElementsInMatrix, int numberOfSlaves, int *mappers, int *reducers, long long int *inputSplit, int *equalDivisionPossible, int *remainingSplit)
{
    // long long int totalElementsInMatrix = sizeOfMatrix * sizeOfMatrix;
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
    matrixIndex* inputMatrixes;
    matrixIndex* resultantMatrix;
    long long int totalIndexes;
    long long int inputSplit = 0;
    int remainingSplit = 0;

    if (isMaster(rank)) // master node
    {
        printf("Master with running id %d running on %s\n", rank, processorName);

        int power = atoi(argv[1]); //maximum is 2^30
        //randomly calculating matrix size
        // int power = 3;//rand() % 12 + (30-12) ;
        // sizeOfMatrix = pow(2, rand() % 20 + 12); // assumed size of matrix
        printf("Power: %d\n",power);
        int sizeOfMatrix = pow(2,power);
        printf("Matrix Size: %d\n",sizeOfMatrix);

        // Read Matrix
        char file1[] = "../datasets/matrixA";
        char file2[] = "../datasets/matrixB";
        
        // list of MatrixIndex struct
        // This is list of both input matrixes in index wise format
        totalIndexes = sizeOfMatrix * sizeOfMatrix;
        inputMatrixes = (matrixIndex*) malloc(sizeof(matrixIndex) * (totalIndexes * 2)) ;

        // list of MatrixIndex struct
        // This is list of resultant matrix in index wise format
        resultantMatrix = (matrixIndex*) malloc(sizeof(matrixIndex) * totalIndexes);

        // reading matrixes from files
        // WARNING: MAKE SURE the FILES ARE ALREADY CREATED AND HAVE CORRECT DATA
        read_matrix_indexWise(file1, file2, inputMatrixes, totalIndexes * 2);

        // printMatrixIndexWise(inputMatrixes, totalIndexes * 2);

        inputSplit = 0;
        remainingSplit = 0;
        calculateNumberOfMappersAndReducers(totalIndexes, numberOfProcesses - 1, &mappers, &reducers, &inputSplit, &equalDivisionPossible, &remainingSplit);

        printf("Size of 2 matrixes: %lld\n", totalIndexes);
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

        // It was done above and we have inputSplit and remainingSplit

        MPI_Barrier(MPI_COMM_WORLD); // task distribution completed

        // send data to mappers here use blocking send
        printf("Sending data to mappers\n");
        send_data_to_mappers(inputMatrixes, totalIndexes * 2, inputSplit, remainingSplit, mappers);
        printf("Data Sent to mappers\n");

        MPI_Barrier(MPI_COMM_WORLD); // Mappers completed processing

        printf("Data Processed by ALL mappers\n");

        // TODO: shuffle here and see what to do for synchronization

        // TODO: send data to reducers here use blocking send

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

        recieve_data_from_master(inputMatrixes, totalIndexes * 2);
        // loop for processing here

        printf("Mapper with running id %d running on %s and mapper rank %d is processing data \n", rank, processorName, mapperRankInMapperComm);
        // printf("From %lld to %lld\n", mapperRankInMapperComm * inputSplit, (mapperRankInMapperComm + 1) * inputSplit - 1);

        // TODO: here map will be called

        MPI_Barrier(MPI_COMM_WORLD); // all mappers completed processing it is ok to shuffle

        // TODO: here key value pairs will be sent to reducers or master

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

        // TODO: receive data from master here use blocking recieve

        // TODO: loop for processing here

        MPI_Barrier(MPI_COMM_WORLD); // all reducers completed processing
    }

    MPI_Finalize(); // finalize MPI environment
}
void read_matrix_indexWise(char* filename1, char* filename2, matrixIndex *matrix, int totalIndexes) {

    // Open the matrix A file for reading
   FILE* fp = fopen(filename1, "r");

    // Read The matrix in this formate name,i,j,value
    int a,b,value;
    char name;
    int i=0;
    for(i=0;i<totalIndexes/2;i++)
    {
        fscanf(fp, "%c,%d,%d,%d\n", &name,&a,&b,&value);

        matrix[i].name = name;
        matrix[i].i = a;
        matrix[i].j = b;
        matrix[i].value = value;

    }
    fclose(fp);

    // Open the matrix B file for reading
    fp = fopen(filename2, "r");

    // Read The matrix in this formate name,i,j,value
    for( ;i<totalIndexes;i++)
    {
        fscanf(fp, "%c,%d,%d,%d\n", &name,&a,&b,&value);

        matrix[i].name = name;
        matrix[i].i = a;
        matrix[i].j = b;
        matrix[i].value = value;
    }
    fclose(fp);

}

void printMatrixIndexWise(matrixIndex *matrix, int totalIndexes)
{
    int i,j;
    for(i=0;i<totalIndexes;i++)
    {
        printf("%c,%d,%d,%d", matrix[i].name, matrix[i].i, matrix[i].j, matrix[i].value);
        printf("\n");
    }
}

void send_data_to_mappers(matrixIndex *inputMatrixes,int totalIndexes, long long int  inputSplit, int remainingSplit, int mappers)
{
    // for all process select mappers and send data to them 
    int i;
    for(i=1;i<=mappers;i++)
    {
        int mapperRank = i;
        int mapperIndex = mapperRank - 1;

        // compute start and end index for mapper
        int mapperStartIndex = mapperIndex * inputSplit;
        int mapperEndIndex = mapperStartIndex + inputSplit - 1;

        // if last mapper then add remaining split
        if(mapperRank == mappers-1)
        {
            mapperEndIndex += remainingSplit;
        }
        // compute data size
        int mapperDataSize = (mapperEndIndex - mapperStartIndex + 1) * sizeof(matrixIndex);
        // send data size
        MPI_Send(&mapperDataSize, 1, MPI_INT, mapperRank, 0, MPI_COMM_WORLD);
        // send data
        MPI_Send(&inputMatrixes[mapperStartIndex], mapperDataSize, MPI_BYTE, mapperRank, 0, MPI_COMM_WORLD);
    }
}
void recieve_data_from_mappers(matrixIndex *inputMatrixes,int totalIndexes, long long int  inputSplit, int remainingSplit, int mappers)
{
    
}
void recieve_data_from_master(matrixIndex *inputMatrixes,int* totalIndexes)
{

    int totalDataSize = 0;
    // wait for data Size from master
    MPI_Recv(&totalDataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    totalIndexes = totalDataSize / sizeof(matrixIndex);

    // Allocate memory for input matrixes
    inputMatrixes = (matrixIndex *)malloc(totalDataSize);

    // wait for data from master
    MPI_Recv(inputMatrixes, totalDataSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}