/***************************************
 * Implementation: HashTable.c
 ***************************************/

/*
 * HashTable.c - Implementation File
 * ----------------------------------
 * Implements the HashTable module defined in HashTable.h.
 */

#include "HashTable.h"
#include <stdlib.h>
#include <string.h>

// Node structure for separate chaining
typedef struct HashNode {
    char *key;
    void *value;
    struct HashNode *next;
} *HashNode;

// HashTable structure
struct HashTable {
    size_t capacity; // Total capacity of the hash table
    size_t size;     // Current number of elements in the hash table
    HashNode *buckets; // Array of bucket pointers
};

// Hash function (djb2 algorithm)
static unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Helper function to resize the hash table
static int resize_table(HashTable table) {
    size_t new_capacity = table->capacity * 2;
    HashNode *new_buckets = calloc(new_capacity, sizeof(HashNode));
    if (!new_buckets) return -1;

    for (size_t i = 0; i < table->capacity; ++i) {
        HashNode node = table->buckets[i];
        while (node) {
            HashNode next = node->next;
            unsigned long new_index = hash(node->key) % new_capacity;
            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            node = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;
    return 0;
}

HashTable HashTable_create(size_t capacity) {
    if (capacity == 0) return NULL;

    HashTable table = malloc(sizeof(struct HashTable));
    if (!table) return NULL;

    table->capacity = capacity;
    table->size = 0;
    table->buckets = calloc(capacity, sizeof(HashNode));
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    return table;
}

void HashTable_destroy(HashTable table) {
    if (!table) return;

    for (size_t i = 0; i < table->capacity; ++i) {
        HashNode node = table->buckets[i];
        while (node) {
            HashNode next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }

    free(table->buckets);
    free(table);
}

int HashTable_add(HashTable table, const char *key, void *value) {
    if (!table || !key) return -1;

    // Resize if load factor exceeds 0.75
    if ((double)table->size / table->capacity >= 0.75) {
        if (resize_table(table) != 0) return -1;
    }

    unsigned long index = hash(key) % table->capacity;
    HashNode node = table->buckets[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value; // Update value if key exists
            return 0;
        }
        node = node->next;
    }

    node = malloc(sizeof(struct HashNode));
    if (!node) return -1;

    node->key = strdup(key);
    if (!node->key) {
        free(node);
        return -1;
    }

    node->value = value;
    node->next = table->buckets[index];
    table->buckets[index] = node;
    table->size++;

    return 0;
}

void *HashTable_get(const HashTable table, const char *key) {
    if (!table || !key) return NULL;

    unsigned long index = hash(key) % table->capacity;
    HashNode node = table->buckets[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value; // Return value if key matches
        }
        node = node->next;
    }

    return NULL; // Key not found
}

int HashTable_remove(HashTable table, const char *key) {
    if (!table || !key) return -1;

    unsigned long index = hash(key) % table->capacity;
    HashNode node = table->buckets[index];
    HashNode prev = NULL;

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                table->buckets[index] = node->next;
            }
            free(node->key);
            free(node);
            table->size--;
            return 0; // Key removed successfully
        }
        prev = node;
        node = node->next;
    }

    return -1; // Key not found
}