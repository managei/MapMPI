#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

// TO COMPILE: 
// gcc createMatrix.c -lm -o createMatrix

// TO RUN:
// ./createMatrix 3

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

    // now we will create a file
    // append datasets directory to file name in front 
    char file1[] = "../datasets/matrixA-";
    char file2[] = "../datasets/matrixB-";

    // strncat(filename, &power, powerSize);
    char powerStr[10];
    sprintf(powerStr, "%d", power);
    strncat(file1, powerStr, strlen(powerStr));
    strncat(file2, powerStr, strlen(powerStr));
    
    // char *file1 = argv[2];
    // char *file2 = argv[3];
    
    printf("File 1 name: %s\n",file1);
    printf("File 2 name: %s\n",file2);

    writeMatrixToFile(matrixSize,file1);
    writeMatrixToFile(matrixSize,file2);

    return 0;
}