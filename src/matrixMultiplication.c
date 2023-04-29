#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

// TO COMPILE: 
// gcc matrixMultiplication.c -lm -o matrixMultiplication

// TO RUN: only the size of matrix : should already be created
// ./matrixMultiplication 3

// This method changes the custom format matrix to 2d-matrix
void read_matrix_indexWise(char* filename, int rows, int cols, int **matrix, int matrixSize) {
    // Open the file for reading
   FILE* fp = fopen(filename, "r");

    // Read the number of rows and columns
    // fscanf(fp, "%d %d", &rows, &cols);
    // printf("Rows: %d\nCols: %d\n",rows,cols);

    // Read The matrix in this formate name,i,j,value
    int i,j,value;
    char name;
    for(i=0;i<matrixSize;i++)
    {
        for(j=0;j<matrixSize;j++)
        {
            fscanf(fp, "%c,%d,%d,%d", &name,&i,&j,&value);
            matrix[i][j] = value;
        }
    }
    fclose(fp);

}

void read_matrix(char* filename, int rows, int cols, int **matrix, int matrixSize) {
    // Open the file for reading
   FILE* fp = fopen(filename, "r");

    // Read the number of rows and columns
    // fscanf(fp, "%d %d", &rows, &cols);
    // printf("Rows: %d\nCols: %d\n",rows,cols);

    // Read the matrix elements
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) 
            {
            // if (fscanf(fp, "%d ", &matrix[i][j]) != 1) {
            //     //fprintf(stderr, "Error: could not read element (%d, %d) from file %s\n", i, j, filename);
            //     //return -1;
            // }
            
                fscanf(fp, "%d ", &matrix[i][j]);
                //printf("%d ",matrix[i][j]);
            }    
            fscanf(fp,"\n");
            //printf("\n");
        
    }
    fclose(fp);
}

void printMatrix(int **matrix, int matrixSize, int no)
{
    printf("Matrix no:%d\n",no);
    int i,j;
    for(i=0;i<matrixSize;i++)
    {
        for(j=0;j<matrixSize;j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }
}

void serialMultiply(int **matA, int**matB, int **matC, int N)
{
    int i, j, k;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matC[i][j] = 0;
            for (k = 0; k < N; k++) {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
        printf("Row %d of %d \n",i,N);
    }
}

int main(int argc, char* argv[]) {

    srand(time(NULL));
    if (argc != 2)
    {
        printf("Please Provid Valid Arguments: \n 1. Power of 2 for matrix size \n");
        return -1;
    }

    int power = atoi(argv[1]); //maximum is 2^30
    //randomly calculating matrix size
    // int power = 3;//rand() % 12 + (30-12) ;
    printf("Power: %d\n",power);
    int matrixSize = pow(2,power);
    printf("Matrix Size: %d\n",matrixSize);


    // append datasets directory to file name in front 
    char file1[] = "../datasets/matrixA";
    char file2[] = "../datasets/matrixB";

    // char powerStr[10];
    // sprintf(powerStr, "%d", power);
    // strncat(file1, powerStr, strlen(powerStr));
    // strncat(file2, powerStr, strlen(powerStr));

    int** matA = (int**) calloc(matrixSize, sizeof(int*));
    int** matB = (int**) calloc(matrixSize, sizeof(int*));
    int** matC = (int**) calloc(matrixSize, sizeof(int*));

    for(int i=0; i<matrixSize; i++)
    {
        matA[i] = (int*)calloc(matrixSize, sizeof(int));
        matB[i] = (int*)calloc(matrixSize, sizeof(int));
        matC[i] = (int*)calloc(matrixSize, sizeof(int));
    }
    printf("----------------------------------------------------------------------------\n");
    printf("----------------------First Matrix------------------------------------------\n");
    printf("----------------------------------------------------------------------------\n");
    read_matrix(file1,matrixSize,matrixSize,matA,matrixSize);
    //printMatrix(matA,matrixSize,1);
    printf("----------------------------------------------------------------------------\n");
    printf("---------------------Second Matrix------------------------------------------\n");
    printf("----------------------------------------------------------------------------\n");
    read_matrix(file2,matrixSize,matrixSize,matB,matrixSize);
    printf("----------------------------------------------------------------------------\n");
    printf("-------------------------------A*B------------------------------------------\n");
    printf("----------------------------------------------------------------------------\n");
    serialMultiply(matA,matB,matC,matrixSize);
    printMatrix(matC,matrixSize,3);

    return 0;
}