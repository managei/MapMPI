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
    char* key;
    char name;
    int i;
    int j;
    int value;
} matrixIndex;



// This method changes the custom format matrix to 2d-matrix
void read_matrix_indexWise(char* filename1, char* filename2, matrixIndex *matrix, int inputSize);

// This method print matrix in index wise format 
void printMatrixIndexWise(matrixIndex *matrix, long long int inputSize);

// This method send data from master to mappers
void send_data_to_mappers(matrixIndex *inputMatrixes,int inputSize, long long int  inputSplit, int remainingSplit,int mappers);

// This method recieve data from master to mappers
long long int recieve_data_from_master(matrixIndex *inputMatrixes, long long int inputSize);

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
    long long int inputSize;
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
        sizeOfMatrix = pow(2,power);
        printf("Matrix Size: %d\n",sizeOfMatrix);

        // Read Matrix
        char file1[] = "../datasets/matrixA";
        char file2[] = "../datasets/matrixB";
        
        // list of MatrixIndex struct
        // This is list of both input matrixes in index wise format
        inputSize = sizeOfMatrix * sizeOfMatrix;
        inputMatrixes = (matrixIndex*) malloc(sizeof(matrixIndex) * (inputSize * 2)) ;

        // list of MatrixIndex struct
        // This is list of resultant matrix in index wise format
        resultantMatrix = (matrixIndex*) malloc(sizeof(matrixIndex) * inputSize);

        // reading matrixes from files
        // WARNING: MAKE SURE the FILES ARE ALREADY CREATED AND HAVE CORRECT DATA
        read_matrix_indexWise(file1, file2, inputMatrixes, inputSize * 2);

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
    MPI_Comm MPI_COMM_MAPPER;    // mapper communicator
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

        HashTable **localMapperHash = (HashTable *)malloc(sizeof(HashTable) * mappers);

        // TODO: shuffle here and see what to do for synchronization
        recieve_data_from_mappers(&localMapperHash, mappers);

        printf("Data Gathered by ALL mappers to Master\n");

        // printMatrixIndexWise(inputMatrixes, inputSize * 2);

        // TODO: send data to reducers here use blocking send

        MPI_Barrier(MPI_COMM_WORLD); // Reducers completed processing
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

        // loop for processing here
        // generate key pairs
        HashTable *localMapperHash = (HashTable *)malloc(sizeof(HashTable));

        create_table(localMapperHash);

        // TODO: here map will be called
        // still needs to save keys
        // printf("Size of Matrix: %lld\n", sizeOfMatrix);
        // printf("Size of recieved matrixes: %lld\n", inputSize);
        for(int i = 0 ; i< inputSize; i++)
        {
            map(inputMatrixes[i], &localMapperHash, sizeOfMatrix);
        }

        MPI_Barrier(MPI_COMM_WORLD); // all mappers completed processing it is ok to shuffle

        // TODO: here key value pairs will be sent to reducers or master
        // send data
        // print_table(localMapperHash);
        // printf("Data Sent by Mapper %d\n", rank);
        // printf("Data Pointer %p\n", localMapperHash);

        // send data size to master
        int size = calculate_size_of_ht(localMapperHash);
        printf("Sending data Size : %d to master by Mapper %d\n", size, rank);

        MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        printf("Sending data to master by Mapper %d\n", rank);

        char* buffer = (char*) malloc(size);
        serialize_ht(buffer, localMapperHash);
        MPI_Send(buffer, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);


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

        // TODO: receive data from master here use blocking recieve

        // TODO: loop for processing here


    }

    MPI_Finalize(); // finalize MPI environment
}
void read_matrix_indexWise(char* filename1, char* filename2, matrixIndex *matrix, int inputSize) {

    // Open the matrix A file for reading
   FILE* fp = fopen(filename1, "r");

    // Read The matrix in this formate name,i,j,value
    int a,b,value;
    char name;
    int i=0;
    for(i=0;i<inputSize/2;i++)
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
    for( ;i<inputSize;i++)
    {
        fscanf(fp, "%c,%d,%d,%d\n", &name,&a,&b,&value);

        matrix[i].name = name;
        matrix[i].i = a;
        matrix[i].j = b;
        matrix[i].value = value;
    }
    fclose(fp);

}

void printMatrixIndexWise(matrixIndex *matrix, long long int inputSize)
{
    int i,j;
    for(i=0;i<inputSize;i++)
    {
        printf("%c,%d,%d,%d", matrix[i].name, matrix[i].i, matrix[i].j, matrix[i].value);
        printf("\n");
    }
}

void send_data_to_mappers(matrixIndex *inputMatrixes,int inputSize, long long int  inputSplit, int remainingSplit, int mappers)
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
void recieve_data_from_mappers(HashTable **hashTables, int mappers)
{
    // for all process select mappers and send data to them 
    int i;
    for(i=1;i<=mappers;i++)
    {
        int mapperRank = i;
        int mapperIndex = mapperRank - 1;

        // get data size for mapper
        int size;
        MPI_Recv(&size, 1, MPI_INT, mapperRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("Recieving data Size : %d from mapper %d\n", size, mapperRank);
        char* buffer = (char*) malloc(size);
        
        printf("Recieving data from mapper %d\n", mapperRank);
        MPI_Recv(buffer, size, MPI_BYTE, mapperRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        HashTable ht;
        create_table(&ht);
        deserialize_ht(buffer, &ht);

        print_table(&ht);
        
        hashTables[mapperIndex] = &ht;
    }
    
}
void send_data_to_master(matrixIndex *inputMatrixes,long long int inputSize)
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

void map(matrixIndex input, HashTable *hash, int sizeOfMatrix)
{   
    char *inputSplit[MAX_LINE_LEN];
    char *outputKey[MAX_LINE_LEN];
    char *outputValue[MAX_LINE_LEN];
    char matrixType;
    int row, col, val, k, i, N = sizeOfMatrix;

    // WE DONT NEED TO SPLIT THE INPUT AS WE ARE USING (STRUCTURE)
    // Parse input matrix A or B
    // int num_splits = 0;
    // inputSplit[num_splits++] = strtok(value, ",");
    // while (num_splits < 4 && (inputSplit[num_splits++] = strtok(NULL, ",")))
    //     ;
    // matrixType = inputSplit[0];
    // row = atoi(inputSplit[1]);
    // col = atoi(inputSplit[2]);
    // val = atoi(inputSplit[3]);
    matrixType = input.name;
    row = input.i;
    col = input.j;
    val = input.value;

    // Emit intermediate key-value pairs
    if (matrixType =='A')
    {
        // printf("Matrix A detected\n");
        // printf("Key\tValue\n");
        for (i = 0; i < N; i++)
        {
            sprintf(outputKey, "%d,%d", row, i);
            sprintf(outputValue, "%c,%d,%d,%d", matrixType, row, col, val);
            // printf("%s\t%s\n", outputKey, outputValue);

            // add this key value to hash map
            insert(hash, outputKey, outputValue);

            // writing this to a file
            // writeStringsToFile(outputKey,outputValue);
            // writeKeysToFile(outputKey);     // key written here
            // writeValuesToFile(outputValue); // value written here
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

            // add this key value to hash map
            insert(hash, outputKey, outputValue);

            // writeStringsToFile(outputKey,outputValue);
            // writeKeysToFile(outputKey);     // key written here
            // writeValuesToFile(outputValue); // value written here
        }
    }
}