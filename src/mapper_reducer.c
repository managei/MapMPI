#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// input to mapper
// MatrixType,Row,Column,Value
// convert into string

int mapperNumber;
int reducerNumber;
bool isNew;

#define MAX_LINE_LEN 1024
#define MAX_LINES 100
#define MAX_LINE_LENGTH 100
int globalN;

void writeStringsToFile(char *str1, char *str2)
{
    FILE *fp;
    fp = fopen("../datasets/mapped", "a");
    fprintf(fp, "%s\t%s\n", str1, str2);
    fclose(fp);
}

void writeKeysToFile(char *str)
{
    FILE *fp;
    if (isNew)
    {
        fp = fopen("../datasets/mapped-key.txt", "w");
        fprintf(fp, "%s\n", str);
    }
    else
    {
        fp = fopen("../datasets/mapped-key.txt", "a");
        fprintf(fp, "%s\n", str);
    }
    fclose(fp);
}

void writeValuesToFile(char *str1)
{
    FILE *fp;
    if (isNew)
    {
        fp = fopen("../datasets/mapped-value.txt", "w");
    }
    else
    {
        fp = fopen("../datasets/mapped-value.txt", "a");
    }
    fprintf(fp, "%s\n", str1);
    fclose(fp);
}

void map(char *key, char *value)
{
    char *inputSplit[MAX_LINE_LEN];
    char *outputKey[MAX_LINE_LEN];
    char *outputValue[MAX_LINE_LEN];
    char *matrixType;
    int row, col, val, k, i, N = globalN;

    // Parse input matrix A or B
    int num_splits = 0;
    inputSplit[num_splits++] = strtok(value, ",");
    while (num_splits < 4 && (inputSplit[num_splits++] = strtok(NULL, ",")))
        ;

    matrixType = inputSplit[0];
    row = atoi(inputSplit[1]);
    col = atoi(inputSplit[2]);
    val = atoi(inputSplit[3]);

    // Emit intermediate key-value pairs
    if (strcmp(matrixType, "A") == 0)
    {
        // printf("Matrix A detected\n");
        // printf("Key\tValue\n");
        for (k = 0; k < N; k++)
        {
            sprintf(outputKey, "%d,%d", row, k);
            sprintf(outputValue, "%s,%d,%d,%d", matrixType, row, col, val);
            printf("%s\t%s\n", outputKey, outputValue);
            // writing this to a file
            // writeStringsToFile(outputKey,outputValue);
            writeKeysToFile(outputKey);     // key written here
            writeValuesToFile(outputValue); // value written here
            isNew = false;
        }
    }
    else
    {
        // printf("Matrix B detected\n");
        // printf("Key\tValue\n");
        for (i = 0; i < N; i++)
        {
            sprintf(outputKey, "%d,%d", i, col);
            sprintf(outputValue, "%s,%d,%d,%d", matrixType, row, col, val);
            printf("%s\t%s\n", outputKey, outputValue);
            // send to the master
            // use appropriate MPI function
            // writeStringsToFile(outputKey,outputValue);
            writeKeysToFile(outputKey);     // key written here
            writeValuesToFile(outputValue); // value written here
            isNew = false;
        }
    }
}

void reduce(char *key, char **values, int num_values)
{
    // Multiply and accumulate the corresponding entries from input matrices A and B
    // printf("Reached here0\n");
    int A[num_values];
    int B[num_values];
    // printf("Reached here1\n");
    for (int i = 0; i < num_values; i++)
    {
        // printf("Printing in loop %d\n", i);
        char *value = strdup(values[i]);
        printf("Value: %s\n", value);
        char *valueSplit[MAX_LINE_LEN];
        int num_splits = 0;
        // printf("Reached here in loop 10\n");
        valueSplit[num_splits++] = strtok(value, ",");
        // printf("Reached here in loop 11\n");
        while (num_splits < 4 && (valueSplit[num_splits++] = strtok(NULL, ",")))
            ;
        printf("%s,%s,%s,%s\n", valueSplit[0], valueSplit[1], valueSplit[2], valueSplit[3]);
        int index = atoi(valueSplit[1]) + atoi(valueSplit[2]);
        int val = atoi(valueSplit[3]);
        // printf("Reached here in loop 12\n");
        if (strcmp(valueSplit[0], "A") == 0)
        {
            A[index] = val;
            printf("A[%d]-> %d\n", index, A[index]);
        }
        else
        {
            B[index] = val;
            printf("B[%d]-> %d\n", index, B[index]);
        }
    }
    // printf("Reached here2\n");
    int dotProduct = 0;
    for (int i = 0; i < globalN; i++)
    {
        dotProduct += A[i] * B[i];
    }
    // printf("Reached here3\n");
    //  Emit output key-value pairs
    char *keySplit[MAX_LINE_LEN];
    int num_splits = 0;
    // printf("Reached here4\n");
    char key_copy[MAX_LINE_LEN];
    strcpy(key_copy, key);
    keySplit[num_splits++] = strtok(key_copy, ",");
    // printf("Reached here5\n");
    while (num_splits < 2 && (keySplit[num_splits++] = strtok(NULL, ",")))
        ;
    int row = atoi(keySplit[0]);
    int col = atoi(keySplit[1]);
    printf("%d,%d,%d\n", row, col, dotProduct);
    // printf("Reached here6\n");
}

/*
void reduce(char* key, char** values, int num_values) {
    // Multiply and accumulate the corresponding entries from input matrices A and B
    int A[N] = {0};
    int B[N] = {0};
    for (int i = 0; i < num_values; i++) {
        char* value = values[i];
        char* valueSplit[MAX_LINE_LEN];
        int num_splits = 0;
        valueSplit[num_splits++] = strtok(value, ",");
        while (num_splits < 4 && (valueSplit[num_splits++] = strtok(NULL, ",")));
        int index = atoi(valueSplit[1]);
        int val = atoi(valueSplit[2]);
        if (strcmp(valueSplit[0], "A") == 0) {
            A[index] = val;
        } else {
            B[index] = val;
        }
    }

    int dotProduct = 0;
    for (int i = 0; i < N; i++) {
        dotProduct += A[i] * B[i];
    }

    // Emit output key-value pairs
    char* keySplit[MAX_LINE_LEN];
    int num_splits = 0;
    keySplit[num_splits++] = strtok(key, ",");
    while (num_splits < 2 && (keySplit[num_splits++] = strtok(NULL, ",")));
    int row = atoi(keySplit[0]);
    int col = atoi(keySplit[1]);
    printf("%d,%d,%d\n", row, col, dotProduct);
}
*/

int read_file(char *filename, char **lines)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int num_lines = 0;

    while (fgets(line, MAX_LINE_LENGTH, fp))
    {
        line[strcspn(line, "\r\n")] = 0;                     // remove newline characters from end of line
        lines[num_lines] = (char *)malloc(strlen(line) + 1); // allocate memory for line
        strcpy(lines[num_lines], line);
        num_lines++;

        if (num_lines == MAX_LINES)
        {
            printf("Maximum number of lines exceeded\n");
            break;
        }
    }

    fclose(fp);
    return num_lines;
}

int countLines(char *filename)
{
    int count = 0;
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        char c;
        while ((c = fgetc(file)) != EOF)
        {
            if (c == '\n')
            {
                count++;
            }
        }
        fclose(file);
    }
    else
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }
    return count;
}

void printLines(char **lines, int lineCount)
{
    for (int i = 0; i < lineCount; i++)
    {
        printf("%s\n", lines[i]);
    }
}

int main()
{
    char file1[50] = "../datasets/matrixA-"; // file1 path
    char file2[50] = "../datasets/matrixB-"; // file2 path

    char *dummyKey = "dummy_key";      // dummy key to be sent to the map function
    int lineCount = countLines(file1); // counting the number of lines in the file
    printf("Lines Counted: %d\n", lineCount);

    int KeyValueCount = 2 * (2 * lineCount); // number of key value pairs
    char *key[KeyValueCount];                // array for keys
    char *value[KeyValueCount];              // array for values

    char *lines1[lineCount]; // creating the char array of size counted before
    char *lines2[lineCount];

    lineCount = read_file(file1, lines1); // reading the file1 into the lines1 array for matrixA
    lineCount = read_file(file2, lines2); // reading the file2 into the lines2
    globalN = sqrt(lineCount);            // return to Fasih

    printLines(lines1, lineCount);
    printf("--------\n");
    printLines(lines2, lineCount);
    printf("--------\n");
    printf("Key\tValue\n");
    for (int i = 0; i < lineCount; i++)
    {
        map(dummyKey, lines1[i]);
        map(dummyKey, lines2[i]);
    }

    printf("---------------\n");
    // printf("Key and Value: %d\n",KeyValueCount);

    KeyValueCount = read_file("../datasets/mapped-key", key);
    KeyValueCount = read_file("../datasets/mapped-value", value);

    // printf("New\nKey and Value: %d\n",KeyValueCount);
    // for(int i=0;i<KeyValueCount;i++)
    // {
    //     printf("%s\t%s\n",key[i],value[i]);
    // }

    printf("Calling reducer\n");
    printf("Matrix,row index,col index,value\n");
    char *mykey = "0,0";
    char *values[] = {"A,0,0,1", "B,0,0,2", "A,0,1,3", "B,1,0,4"};
    int num_values = 4;
    reduce(mykey, values, num_values);
    // mapper receives the list of key-value pairs
    // make a dictionary for the received key-value pair

    // the main process will shuffle the received input

    // the main mappcall reducer

    return 0;
}