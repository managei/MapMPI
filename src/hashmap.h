#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TABLE_SIZE 10000

typedef struct Node
{
    char *key;
    char *value;
    struct Node *next;
} Node;

typedef struct HashTable
{
    Node **table;
} HashTable;

// TODO: Fix hash method : make it for unique indexes, now (,) in string is a problem
// use i and j
unsigned int hash(char *key)
{
    unsigned int hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % TABLE_SIZE;
}

void create_table(HashTable *ht)
{
    ht->table = (Node **)malloc(sizeof(Node *) * TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        ht->table[i] = NULL;
    }
}

void insert(HashTable *ht, char *key, char *value)
{
    unsigned int index = hash(key);
    Node *node = ht->table[index];
    // printf("Index: %d\n", index);
    // printf("Key: %s\n", key);
    // printf("Value: %s\n", value);

    // Check if key already exists in table
    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            // Key already exists, add new value to end of list
            char *new_value = (char *)malloc(strlen(value) + 1);
            strcpy(new_value, value);
            Node *new_node = (Node *)malloc(sizeof(Node));
            new_node->key = node->key;
            new_node->value = new_value;
            new_node->next = node->next;
            node->next = new_node;
            // printf("hit (%s : %s) \n", new_node->key, new_node->value);
            return;
        }
        node = node->next;
    }

    // If key is not found, create new node
    char *new_key = (char *)malloc(strlen(key) + 1);
    strcpy(new_key, key);
    char *new_value = (char *)malloc(strlen(value) + 1);
    strcpy(new_value, value);
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->key = new_key;
    new_node->value = new_value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;

    // printf("miss (%s : %s) \n", new_node->key, new_node->value);
}

char *get(HashTable *ht, char *key)
{
    unsigned int index = hash(key);
    Node *node = ht->table[index];

    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            return node->value;
        }
        node = node->next;
    }

    return -1; // Key not found
}

void delete(HashTable *ht, char *key)
{
    unsigned int index = hash(key);
    Node *node = ht->table[index];
    Node *prev = NULL;

    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            if (prev == NULL)
            {
                ht->table[index] = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void print_table(HashTable *ht)
{
    printf("Hash Table:\n");

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node *node = ht->table[i];
        if (node != NULL)
        {
            printf("Index %d:", i);
            printf("\n");
        }

        while (node != NULL)
        {
            if (node->key == NULL || node->value == NULL)
            {
                printf("NULL");
            }
            else
            {
                printf(" (%s : %s)", node->key, node->value);
                node = node->next;
                printf("\n");
            }
        }
    }
}

// Calculate the size of a serialized HashTable
int calculate_size_of_ht(HashTable *ht)
{
    int size = sizeof(HashTable);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node *node = ht->table[i];
        while (node != NULL)
        {
            size += sizeof(Node);
            size += strlen(node->key) + 1;
            size += strlen(node->value) + 1;
            node = node->next;
        }
    }
    return size;
}

// Serialize a HashTable into a buffer
void serialize_ht(char *buffer, HashTable *ht)
{
    size_t offset = 0;
    memcpy(buffer + offset, ht, sizeof(HashTable));
    offset += sizeof(HashTable);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node *node = ht->table[i];
        while (node != NULL)
        {
            // Serialize key
            size_t key_size = strlen(node->key) + 1;
            memcpy(buffer + offset, node->key, key_size);
            offset += key_size;
            // Serialize value
            size_t value_size = strlen(node->value) + 1;
            memcpy(buffer + offset, node->value, value_size);
            offset += value_size;
            node = node->next;
        }
    }
}

// Deserialize a buffer into a HashTable
void deserialize_ht(char *buffer, HashTable *ht)
{
    size_t offset = 0;
    memcpy(ht, buffer + offset, sizeof(HashTable));
    offset += sizeof(HashTable);
    create_table(ht);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node **node_ptr = &ht->table[i];
        while (offset < calculate_size_of_ht(ht))
        {
            // Deserialize key
            char *key = (char *)malloc(strlen((char *)buffer + offset) + 1);
            strcpy(key, (char *)buffer + offset);
            offset += strlen(key) + 1;
            // Deserialize value
            char *value = (char *)malloc(strlen((char *)buffer + offset) + 1);
            strcpy(value, (char *)buffer + offset);
            offset += strlen(value) + 1;
            // Create new node
            Node *node = (Node *)malloc(sizeof(Node));
            node->key = key;
            node->value = value;
            node->next = NULL;
            // Add node to end of linked list
            *node_ptr = node;
            node_ptr = &node->next;
        }
    }
}

char *appendAllKeysToString(HashTable *ht, char *seperator, long long int *resultSize)
{
    long long int size = TABLE_SIZE * 10;
    char *result = (char *)malloc(size);
    long long int actualSize = 0;
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node *node = ht->table[i];
        if (node != NULL)
        {
            strcat(result, node->key);
            strcat(result, seperator);
            actualSize += strlen(node->key) + strlen(seperator);
        }
    }
    result[actualSize] = '\0';
    actualSize += 1;
    *resultSize = actualSize;
    return result;
}

char *appendAllValuesToString(HashTable *ht, char *seperator, char *valueSeperator, long long int *resultSize)
{
    long long int size = TABLE_SIZE * TABLE_SIZE * 4;
    char *result = (char *)malloc(size);
    long long int actualSize = 0;
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node *node = ht->table[i];
        if (node != NULL)
        {
            while (node != NULL)
            {
                strcat(result, node->value);
                strcat(result, valueSeperator);
                actualSize += strlen(node->value) + strlen(seperator);
                node = node->next;
            }
            strcat(result, seperator);
        }
        else
        {
            strcat(result, seperator);
        }
    }
    result[actualSize] = '\0';
    actualSize += 1;
    *resultSize = actualSize;
    return result;
}