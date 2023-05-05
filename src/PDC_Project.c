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
#include "hashmap.h"

#define MAX_LINE_LEN 1024
#define MAX_LINES 100
#define MAX_LINE_LENGTH 100

typedef struct
{
    int keyI;
    int keyJ;
    char name;
    int i;
    int j;
    int value;
} matrixIndex;

// This method changes the custom format matrix to 2d-matrix
void read_matrix_indexWise(char *filename1, char *filename2, matrixIndex *matrix, int inputSize);

// This method print matrix in index wise format
void printMatrixIndexWise(matrixIndex *matrix, long long int inputSize);

// This method send data from master to mappers
void send_data_to_mappers(matrixIndex *inputMatrixes, int inputSize, long long int inputSplit, int remainingSplit, int mappers);

// This method recieve data from master to mappers
long long int recieve_data_from_master(matrixIndex *inputMatrixes, long long int inputSize);

// This method recieves data from mappers to Master
// The data size is not being recieved properly
void recieve_data_from_mappers(matrixIndex *allKeys, int mappers, int size);

// This method sends data from mappers to Master
void send_data_to_master(matrixIndex *inputMatrixes, long long int inputSize);

// This method recieves data from Master to mapper (NOT BEING USED : Using inline code in mapper)
long long int recieve_data_from_master(matrixIndex *inputMatrixes, long long int inputSize);

// This method compares two matrixIndex struct
int compareByKeys(const void *a, const void *b);

// This is map function for mapper
// takes in input the matrix index
// and returns list of keys
void map(matrixIndex input, matrixIndex *keysList, int index, int sizeOfMatrix);

// This is reduce function for reducer
// takes in input the matrix index
// and returns resultant matrix
void reduce(matrixIndex *input, int inputSize, int index, int *resultant, int matrixSize);

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
    matrixIndex *inputMatrixes;
    matrixIndex *resultantMatrix;
    long long int inputSize;
    long long int inputSplit = 0;
    int remainingSplit = 0;

    if (isMaster(rank)) // master node
    {
        printf("Master with running id %d running on %s\n", rank, processorName);

        int power = atoi(argv[1]); // maximum is 2^30
        // randomly calculating matrix size
        //  int power = 3;//rand() % 12 + (30-12) ;
        //  sizeOfMatrix = pow(2, rand() % 20 + 12); // assumed size of matrix
        printf("Power: %d\n", power);
        sizeOfMatrix = pow(2, power);
        printf("Matrix Size: %d\n", sizeOfMatrix);

        // Read Matrix
        char file1[] = "../datasets/matrixA";
        char file2[] = "../datasets/matrixB";

        // list of MatrixIndex struct
        // This is list of both input matrixes in index wise format
        inputSize = sizeOfMatrix * sizeOfMatrix;
        inputMatrixes = (matrixIndex *)malloc(sizeof(matrixIndex) * (inputSize * 2));

        // list of MatrixIndex struct
        // This is list of resultant matrix in index wise format
        resultantMatrix = (matrixIndex *)malloc(sizeof(matrixIndex) * inputSize);

        // reading matrixes from files
        // WARNING: MAKE SURE the FILES ARE ALREADY CREATED AND HAVE CORRECT DATA
        read_matrix_indexWise(file1, file2, inputMatrixes, inputSize * 2);

        // printMatrix2D(inputMatrixes, inputSize * 2, sizeOfMatrix);

        // printMatrixIndexWise(inputMatrixes, inputSize * 2);

        inputSplit = 0;
        remainingSplit = 0;
        calculateNumberOfMappersAndReducers(inputSize * 2, numberOfProcesses - 1, &mappers, &reducers, &inputSplit, &equalDivisionPossible, &remainingSplit);

        printf("Size of Matrix: %lld\n", sizeOfMatrix);
        printf("Size of 2 matrixes: %lld\n", inputSize * 2);
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
    MPI_Comm MPI_COMM_MAPPER;                         // mapper communicator
    MPI_Group MPI_GROUP_MAPPER;
    if (isMaster(rank))
    {
        // task distribution here

        // It was done above and we have inputSplit and remainingSplit

        MPI_Barrier(MPI_COMM_WORLD); // task distribution completed

        // send data to mappers here use blocking send
        printf("Sending data to mappers by Master\n");
        send_data_to_mappers(inputMatrixes, inputSize * 2, inputSplit, remainingSplit, mappers);

        MPI_Barrier(MPI_COMM_WORLD); // Mappers completed processing

        int sizeOfKeys = inputSplit * sizeOfMatrix;
        // printf("Size of keys: %d\n", sizeOfKeys);
        matrixIndex *allKeys = (matrixIndex *)malloc(sizeof(matrixIndex) * mappers * sizeOfKeys);

        recieve_data_from_mappers(allKeys, mappers, sizeOfKeys);
        printf("Reieved keys from mappers to Master\n");

        qsort(allKeys, mappers * sizeOfKeys, sizeof(matrixIndex), compareByKeys);
        printf("Sorted keys by Master\n");

        // All Keys
        // printMatrixIndexWise(allKeys, mappers * sizeOfKeys);

        // send data to reducers here use blocking send
        MPI_Barrier(MPI_COMM_WORLD); // Reducers completed processing

        // All similar keys are together
        // So we can divide the keys equally among reducers
        // We can send the keys to reducers in round robin fashion

        int reducerKeys = (sizeOfMatrix * sizeOfMatrix) / reducers;
        int remainingKeys = (sizeOfMatrix * sizeOfMatrix) % reducers;

        printf("Total Keys: %d\n", mappers * sizeOfKeys);
        printf("Total Reducers: %d\n", reducers);
        printf("Total Keys per reducer: %d\n", reducerKeys);
        printf("Remaining Keys : %d\n", remainingKeys);

        for (int i = 0; i < reducers; i++)
        {
            printf("Sending data to reducer %d\n", i + 1);
            if (i == reducers - 1 && remainingKeys != 0)
                send_data_to_reducers(allKeys, i + 1, mappers, reducers, reducerKeys * (sizeOfMatrix + sizeOfMatrix), (reducerKeys + remainingKeys) * (sizeOfMatrix + sizeOfMatrix));
            else
                send_data_to_reducers(allKeys, i + 1, mappers, reducers, reducerKeys * (sizeOfMatrix + sizeOfMatrix), reducerKeys * (sizeOfMatrix + sizeOfMatrix));
        }

        // recieve data from reducers here use blocking recieve
        int* resultant = (int *)malloc(sizeof(int) * sizeOfMatrix * sizeOfMatrix);
        for (int i = 0; i < reducers; i++)
        {
            printf("Recieving data from reducers %d\n", i + 1);
            if (i == reducers - 1 && remainingKeys != 0)
                recieve_data_from_reducers(resultant, i + 1, mappers, reducers, reducerKeys, reducerKeys + remainingKeys);
            else
                recieve_data_from_reducers(resultant, i + 1, mappers, reducers, reducerKeys, reducerKeys);

        }

        printf("Resultant Matrix\n");
        // printing only first 5 and last 5 elements of each row and columns - head and tail
        for (int i = 0; i < sizeOfMatrix; i++)
        {
            for (int j = 0; j < sizeOfMatrix; j++)
            {
                if (j < 5 || j >= sizeOfMatrix - 5)
                    printf("%d ", resultant[i * sizeOfMatrix + j]);
                else if (j == 5)
                    printf("... ");
            }
            printf("\n");
            if (i == 5)
                printf("...\n");
        }
    }
    else if (isMapper(rank, mappers)) // mapper node code
    {
        MPI_Barrier(MPI_COMM_WORLD); // wait for task distribution

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
        int totalDataSize = 0;
        // wait for data Size from master
        MPI_Recv(&totalDataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        inputSize = totalDataSize / sizeof(matrixIndex);

        // Allocate memory for input matrixes
        inputMatrixes = (matrixIndex *)malloc(totalDataSize);

        // wait for data from master
        MPI_Recv(inputMatrixes, totalDataSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // generate key pairs
        int totalKeys = inputSize * sizeOfMatrix;
        // printf("Total keys: %d\n", totalKeys);
        matrixIndex *localKeys = (matrixIndex *)malloc(sizeof(matrixIndex) * totalKeys);

        // loop for processing here
        for (int i = 0; i < inputSize; i++)
        {
            map(inputMatrixes[i], localKeys, i, sizeOfMatrix);
        }

        MPI_Barrier(MPI_COMM_WORLD); // all mappers completed processing it is ok to shuffle

        // send data size to master
        MPI_Send(localKeys, sizeof(matrixIndex) * totalKeys, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD); // allow reducers to complete processing
    }
    else if (isReducer(rank, mappers)) // reducer node code
    {
        MPI_Barrier(MPI_COMM_WORLD); // wait for task distribution
        MPI_Barrier(MPI_COMM_WORLD); // wait for shuffling
        MPI_Barrier(MPI_COMM_WORLD); // all reducers can recieve now

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
        int totalDataSize = 0;
        // wait for data Size from master
        MPI_Recv(&totalDataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        inputSize = totalDataSize / sizeof(matrixIndex);

        // Allocate memory for input matrixes
        inputMatrixes = (matrixIndex *)malloc(totalDataSize);

        // wait for data from master
        MPI_Recv(inputMatrixes, totalDataSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // print recieved data
        // printMatrixIndexWise(inputMatrixes, inputSize);

        // loop for processing here

        // group by name
        int totalData = totalDataSize / sizeof(matrixIndex);
        int totalKeys = totalData / (sizeOfMatrix * 2);
                
        // result matrix
        int resultantSize = totalKeys;
        int *resultant = (int *)malloc(sizeof(int) * resultantSize);

        for (int i=0; i < resultantSize; i++) {
            resultant[i] = -1;
        }

        // iterate over the sorted elements
        int k = 0;
        int i = 0;
        while (i < totalData) {
            // get the current key
            int currentKeyI = inputMatrixes[i].keyI;
            int currentKeyJ = inputMatrixes[i].keyJ;

            // iterate over the next matrixSize * 2 elements
            int sum = 0;
            for (int j =0 ;  j < sizeOfMatrix; j++) {
                // multiply A and B and add to sum
                sum += inputMatrixes[i + j].value * inputMatrixes[i + j + sizeOfMatrix].value;
                
            }

            // store the sum in the resultant matrix
            resultant[k++] = sum;

            // increment i by matrixSize * 2
            i += sizeOfMatrix * 2;
        }        

        // for (int i = 0; i < resultantSize; i++)
        // {
        //     printf("%d ", resultant[i]);
        //     if ((i + 1) % sizeOfMatrix == 0)
        //         printf("\n");
        // }

        // send resultant array back to master
        MPI_Send(resultant, sizeof(int) * resultantSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize(); // finalize MPI environment
}
// compare by keys
int compareByKeys(const void *a, const void *b)
{
    matrixIndex *matrixIndexA = (matrixIndex *)a;
    matrixIndex *matrixIndexB = (matrixIndex *)b;

    // sort by both keys in ascending order
    if (matrixIndexA->keyI > matrixIndexB->keyI)
        return 1;
    else if (matrixIndexA->keyI < matrixIndexB->keyI)
        return -1;
    else
    {
        if (matrixIndexA->keyJ > matrixIndexB->keyJ)
            return 1;
        else if (matrixIndexA->keyJ < matrixIndexB->keyJ)
            return -1;
        else
            return 0;
    }
}

void read_matrix_indexWise(char *filename1, char *filename2, matrixIndex *matrix, int inputSize)
{

    // Open the matrix A file for reading
    FILE *fp = fopen(filename1, "r");

    // Read The matrix in this formate name,i,j,value
    int a, b, value;
    char name;
    int i = 0;
    for (i = 0; i < inputSize / 2; i++)
    {
        fscanf(fp, "%c,%d,%d,%d\n", &name, &a, &b, &value);

        matrix[i].name = name;
        matrix[i].i = a;
        matrix[i].j = b;
        matrix[i].value = value;
    }
    fclose(fp);

    // Open the matrix B file for reading
    fp = fopen(filename2, "r");

    // Read The matrix in this formate name,i,j,value
    for (; i < inputSize; i++)
    {
        fscanf(fp, "%c,%d,%d,%d\n", &name, &a, &b, &value);

        matrix[i].name = name;
        matrix[i].i = a;
        matrix[i].j = b;
        matrix[i].value = value;
    }
    fclose(fp);
}

void printMatrixIndexWise(matrixIndex *matrix, long long int inputSize)
{
    int i, j;
    for (i = 0; i < inputSize; i++)
    {
        printf("%d,%d: %c %d,%d %d \n", matrix[i].keyI, matrix[i].keyJ, matrix[i].name, matrix[i].i, matrix[i].j, matrix[i].value);
        printf("\n");
    }
}
void printMatrix2D(matrixIndex *matrix, int size, int sizeOfMatrix)
{
    int i, j;
    printf("Matrix A\n");
    for (i = 0; i < size; i++)
    {
        if (matrix[i].name == 'A')
        {
            printf("%d ", matrix[i].value);
            if (matrix[i].j == sizeOfMatrix - 1)
                printf("\n");
        }
    }
    printf("Matrix B\n");
    for (i = 0; i < size; i++)
    {
        if (matrix[i].name == 'B')
        {
            printf("%d ", matrix[i].value);
            if (matrix[i].j == sizeOfMatrix - 1)
                printf("\n");
        }
    }
}

void send_data_to_reducers(matrixIndex *inputMatrixes, int i, int mappers, int reducers, int sizeOfMatrix, int inputSize)
{
    // for all process select reducers and send data to them
    int reducerRank = i + mappers;
    int reducerIndex = reducerRank - mappers - 1;

    // compute start and end index for reducer
    int reducerStartIndex = reducerIndex * sizeOfMatrix;
    int reducerEndIndex = reducerStartIndex + inputSize - 1;

    // compute data size
    int reducerDataSize = (reducerEndIndex - reducerStartIndex + 1) * sizeof(matrixIndex);

    printf("Reducer: %d, Start Index: %d, End Index: %d, Data Size: calc: %d, send: %d\n", reducerRank, reducerStartIndex, reducerEndIndex, inputSize, reducerDataSize);

    // send data size
    MPI_Send(&reducerDataSize, 1, MPI_INT, reducerRank, 0, MPI_COMM_WORLD);
    // send data
    MPI_Send(inputMatrixes + reducerStartIndex, reducerDataSize, MPI_BYTE, reducerRank, 0, MPI_COMM_WORLD);
}
void send_data_to_mappers(matrixIndex *inputMatrixes, int inputSize, long long int inputSplit, int remainingSplit, int mappers)
{
    // for all process select mappers and send data to them
    int i;
    for (i = 1; i <= mappers; i++)
    {
        int mapperRank = i;
        int mapperIndex = mapperRank - 1;

        // compute start and end index for mapper
        int mapperStartIndex = mapperIndex * inputSplit;
        int mapperEndIndex = mapperStartIndex + inputSplit - 1;

        // if last mapper then add remaining split
        if (mapperRank == mappers - 1)
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
void recieve_data_from_mappers(matrixIndex *allKeys, int mappers, int size)
{
    // for all process select mappers and send data to them
    int i;
    for (i = 1; i <= mappers; i++)
    {
        int mapperRank = i;
        int mapperIndex = mapperRank - 1;

        matrixIndex *buffer = (matrixIndex *)malloc(sizeof(matrixIndex) * size);

        printf("Recieving data from mapper %d\n", mapperRank);
        MPI_Recv(buffer, sizeof(matrixIndex) * size, MPI_BYTE, mapperRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // printMatrixIndexWise(buffer, size);

        int j;
        for (j = 0; j < size; j++)
        {
            allKeys[mapperIndex * size + j] = buffer[j];
        }
    }
}
void recieve_data_from_reducers(int *resultant, int i, int mappers, int reducers, int sizeOfMatrix, int inputSize)
{
    // for all process select reducers and send data to them
    int reducerRank = i + mappers;
    int reducerIndex = reducerRank - mappers - 1;

    // compute start and end index for reducer
    int reducerStartIndex = reducerIndex * sizeOfMatrix;
    int reducerEndIndex = reducerStartIndex + inputSize - 1;

    // compute data size
    int reducerDataSize = (reducerEndIndex - reducerStartIndex + 1) * sizeof(int);

    printf("Reducer: %d, Start Index: %d, End Index: %d, Data Size: calc: %d, send: %d\n", reducerRank, reducerStartIndex, reducerEndIndex, inputSize, reducerDataSize);

    // send data size
    MPI_Recv(resultant + reducerStartIndex, reducerDataSize, MPI_BYTE, reducerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
void send_data_to_master(matrixIndex *inputMatrixes, long long int inputSize)
{
    int totalDataSize = inputSize * sizeof(matrixIndex);
    // send data
    MPI_Send(inputMatrixes, totalDataSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
}

long long int recieve_data_from_master(matrixIndex *inputMatrixes, long long int inputSize)
{

    int totalDataSize = 0;
    // wait for data Size from master
    MPI_Recv(&totalDataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    inputSize = totalDataSize / sizeof(matrixIndex);

    // Allocate memory for input matrixes
    inputMatrixes = (matrixIndex *)malloc(totalDataSize);

    // wait for data from master
    MPI_Recv(inputMatrixes, totalDataSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // printf("Recieved data from master\n");
    // printf("Data Size %d\n", totalDataSize);

    return inputSize;
}

void map(matrixIndex input, matrixIndex *keysList, int index, int sizeOfMatrix)
{
    char *inputSplit[MAX_LINE_LEN];
    char *outputKey[MAX_LINE_LEN];
    char *outputValue[MAX_LINE_LEN];
    char matrixType;
    int row, col, val, k, i, N = sizeOfMatrix;

    // Get values
    matrixType = input.name;
    row = input.i;
    col = input.j;
    val = input.value;

    // Emit intermediate key-value pairs
    if (matrixType == 'A')
    {
        // printf("Matrix A detected\n");
        // printf("Key\tValue\n");
        for (i = 0; i < N; i++)
        {
            sprintf(outputKey, "%d,%d", row, i);
            sprintf(outputValue, "%c,%d,%d,%d", matrixType, row, col, val);
            // printf("%s\t%s\n", outputKey, outputValue);

            // add this key value to keysList
            keysList[index * N + i].keyI = row;
            keysList[index * N + i].keyJ = i;

            keysList[index * N + i].name = matrixType;
            keysList[index * N + i].i = row;
            keysList[index * N + i].j = col;
            keysList[index * N + i].value = val;
        }
    }
    else
    {
        // printf("Matrix B detected\n");
        // printf("Key\tValue\n");
        for (i = 0; i < N; i++)
        {
            sprintf(outputKey, "%d,%d", i, col);
            sprintf(outputValue, "%c,%d,%d,%d", matrixType, row, col, val);
            // printf("%s\t%s\n", outputKey, outputValue);

            // add this key value to keysList
            keysList[index * N + i].keyI = i;
            keysList[index * N + i].keyJ = col;

            keysList[index * N + i].name = matrixType;
            keysList[index * N + i].i = row;
            keysList[index * N + i].j = col;
            keysList[index * N + i].value = val;
        }
    }
}
