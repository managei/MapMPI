#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void writeMatrixToFile(int matrixSize, char *file)
{
    int i,j,num=0;
    printf("Writing to %s\n",file);
    FILE * fp = fopen(file, "w+"); // open the file for reading

    for(i=0;i<matrixSize;i++)
    {
        for(j=0;j<matrixSize;j++)
        { 
            fprintf(fp,"%d ",num);
            num++;
        }
        fprintf(fp,"\n");
    }
    printf("Data written to %s\n",file);
    fclose(fp);
}

void writeMatrixToFileBinary(int matrixSize, char *file)
{
    int i,j,num=rand() % 5;
    printf("Writing to %s\n",file);
    FILE * fp = fopen(file, "wb"); // open the file for reading

    for(i=0;i<matrixSize;i++)
    {
        for(j=0;j<matrixSize;j++)
        { 
            fwrite(&num,sizeof(int),1,fp);
            num++;
        }
    }
    printf("Data written to %s\n",file);
    fclose(fp);
}

void read_matrix(char* filename, int rows, int cols, int **matrix, int matrixSize) {
    // Open the file for reading
   FILE* fp = fopen(filename, "r");

    // Read the number of rows and columns
//    fscanf(fp, "%d %d", &rows, &cols);
//    printf("Rows: %d\nCols: %d\n",rows,cols);

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
    //int power = atoi(argv[1]); //maximum is 2^30
    //printf("Power: %d\n",power);
    //int matrixSize = 0;//rand() % (1 << 16 - 1 - 12 + 1) + (1 << 12); 

    //randomly calculating matrix size
    int power = 3;//rand() % 12 + (30-12) ;
    printf("Power: %d\n",power);
    int matrixSize = pow(2,power);
    printf("Matrix Size: %d\n",matrixSize);

    //now we will create a file
    char *file1 = argv[1];
    char *file2 = argv[2];
    printf("File 1 name: %s\n",file1);
    printf("File 2 name: %s\n",file2);

    writeMatrixToFile(matrixSize,file1);
    writeMatrixToFile(matrixSize,file2);

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