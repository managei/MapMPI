#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

// TO COMPILE:
// gcc serialMultiplication.c -lm -o serialMultiplication

// TO RUN: only the size of matrix : should already be created
// ./serialMultiplication 3 matrixA matrixB

// This method changes the custom format matrix to 2d-matrix

void read_matrix_indexWise(char *filename1, int **matrix, int inputSize)
{

    // Open the matrix A file for reading
    FILE *fp = fopen(filename1, "r");

    // Read The matrix in this formate name,i,j,value
    int a, b, value;
    char name;
    int i = 0;
    int j = 0;
    for (i = 0; i < inputSize; i++)
    {
        for (j = 0; j < inputSize; j++)
        {
            fscanf(fp, "%c,%d,%d,%d\n", &name, &a, &b, &value);
            matrix[i][j] = value;
        }
    }
    fclose(fp);
}

void read_matrix(char *filename, int **matrix, int matrixSize)
{
    // Open the file for reading
    FILE *fp = fopen(filename, "r");

    // Read the matrix elements
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            fscanf(fp, "%d ", &matrix[i][j]);
        }
        fscanf(fp, "\n");
    }
    fclose(fp);
}

void printMatrix2D(int **matrix, int sizeOfMatrix)
{
    for (int i = 0; i < sizeOfMatrix; i++)
    {
        if (i < 5 || i >= sizeOfMatrix - 5)
        {
            printf("| ");
            for (int j = 0; j < sizeOfMatrix; j++)
            {
                if (j < 5 || j >= sizeOfMatrix - 5)
                    printf("%d ", matrix[i * sizeOfMatrix + j]);
                else if (j == 5)
                    printf("... ");
            }
            printf("\n");
        }
        if (i == 5)
            printf("...\n");
    }
}

void serialMultiply(int **matA, int **matB, int **matC, int N)
{
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            matC[i][j] = 0;
            for (k = 0; k < N; k++)
            {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
        // printf("Row %d of %d \n", i, N);
    }
}

void writeToReport(int proc, long long int sizeOfMatrix, double timeTaken, double Gflops)
{
    FILE *fp;
    fp = fopen("../datasets/report.txt", "a+");
    fprintf(fp, "%d,%lld,%f,%f\n", proc, sizeOfMatrix, timeTaken, Gflops);
    fclose(fp);
}
void write2DMatrix(char *fileName, int **matrix, int sizeOfMatrix)
{
    char path[] = "../datasets/";
    char file[100];
    sprintf(file, "%s%s.txt", path, fileName);

    FILE *fp;
    fp = fopen(file, "w+");
    for (int i = 0; i < sizeOfMatrix; i++)
    {
        for (int j = 0; j < sizeOfMatrix; j++)
        {
            fprintf(fp, "%d ", matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}
int main(int argc, char *argv[])
{

    srand(time(0)); // starting time
    struct timeval start, end;
    int operation_count; // What are the number of operations n = matrix size  // sizeOfMatrix * sizeOfMatrix * sizeOfMatrix

    srand(time(NULL));

    if (argc != 4)
    {
        printf("| Usage: %s <power> <file1> <file2>\n", argv[0]);
        exit(1);
    }

    int power = atoi(argv[1]); // maximum is 2^30
    char *file1 = argv[2];
    char *file2 = argv[3];

    // randomly calculating matrix size
    //  int power = 3;//rand() % 12 + (30-12) ;
    //  sizeOfMatrix = pow(2, rand() % 20 + 12); // assumed size of matrix
    printf("| Power: %d\n", power);
    int sizeOfMatrix = pow(2, power);
    printf("| Matrix Size: %d\n", sizeOfMatrix);
    operation_count = sizeOfMatrix * sizeOfMatrix * sizeOfMatrix;

    // Read Matrix
    // append file name to path
    char path[] = "../datasets/";

    char *file1Path = (char *)malloc(sizeof(char) * (strlen(path) + strlen(file1) + 1));
    strcpy(file1Path, path);
    strcat(file1Path, file1);

    char *file2Path = (char *)malloc(sizeof(char) * (strlen(path) + strlen(file2) + 1));
    strcpy(file2Path, path);
    strcat(file2Path, file2);

    printf("| File 1: %s\n", file1Path);
    printf("| File 2: %s\n", file2Path);

    // char powerStr[10];
    // sprintf(powerStr, "%d", power);
    // strncat(file1, powerStr, strlen(powerStr));
    // strncat(file2, powerStr, strlen(powerStr));

    int **matA = (int **)calloc(sizeOfMatrix, sizeof(int *));
    int **matB = (int **)calloc(sizeOfMatrix, sizeof(int *));
    int **matC = (int **)calloc(sizeOfMatrix, sizeof(int *));

    for (int i = 0; i < sizeOfMatrix; i++)
    {
        matA[i] = (int *)calloc(sizeOfMatrix, sizeof(int));
        matB[i] = (int *)calloc(sizeOfMatrix, sizeof(int));
        matC[i] = (int *)calloc(sizeOfMatrix, sizeof(int));
    }

    read_matrix_indexWise(file1Path, matA, sizeOfMatrix);
    // printMatrix(matA, sizeOfMatrix);
    read_matrix_indexWise(file2Path, matB, sizeOfMatrix);
    // printMatrix(matA, sizeOfMatrix);

    gettimeofday(&start, NULL);                     // starting point 1
    serialMultiply(matA, matB, matC, sizeOfMatrix); // serial multiplication time
    gettimeofday(&end, NULL);                       // end point 1

    double seconds1 = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflops1 = 2e-9 * operation_count / seconds1;

    printMatrix2D(matC, sizeOfMatrix);
    printf("| --------------------GFLOPS REPORT--------------------\n");
    printf("| Time taken to compute matrix C: %f Gflop/s.\n", Gflops1);
    writeToReport(1, sizeOfMatrix, seconds1, Gflops1);
    write2DMatrix("matrixC_S", matC, sizeOfMatrix);
    return 0;

    // gettimeofday(&start, NULL); // starting point 1
    // // second operation called here
    // gettimeofday(&end, NULL); // end point 1
    // double seconds2 = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    // double Gflops1 = 2e-9 * operation_count / seconds2;
}