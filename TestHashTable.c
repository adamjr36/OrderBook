/* TestHashTable.c */
/*
 * Test Suite for HashTable Module
 * This program rigorously tests the functionality of the HashTable module.
 */

#include "HashTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_create_destroy() {
    printf("Testing HashTable_create and HashTable_destroy...\n");
    HashTable table = HashTable_create(10);
    if (!table) {
        printf("Failed to create hash table.\n");
        exit(EXIT_FAILURE);
    }
    HashTable_destroy(&table);
    printf("Passed.\n\n");
}

void test_add_get() {
    printf("Testing HashTable_add and HashTable_get...\n");
    HashTable table = HashTable_create(10);
    if (!table) {
        printf("Failed to create hash table.\n");
        exit(EXIT_FAILURE);
    }

    HashTable_add(table, "key1", "value1");
    HashTable_add(table, "key2", "value2");
    HashTable_add(table, "key3", "value3");

    const char *value = HashTable_get(table, "key1");
    printf("Expected: value1, Got: %s\n", value);

    value = HashTable_get(table, "key2");
    printf("Expected: value2, Got: %s\n", value);

    value = HashTable_get(table, "nonexistent");
    printf("Expected: (null), Got: %s\n\n", value);

    HashTable_destroy(&table);
}

void test_remove() {
    printf("Testing HashTable_remove...\n");
    HashTable table = HashTable_create(10);
    if (!table) {
        printf("Failed to create hash table.\n");
        exit(EXIT_FAILURE);
    }

    HashTable_add(table, "key1", "value1");
    HashTable_add(table, "key2", "value2");

    printf("Getting key1...\n");
    printf("Expected: value1, Got: %s\n", (char *)HashTable_get(table, "key1"));

    printf("Removing key1...\n");
    HashTable_remove(table, "key1");
    printf("Expected: (null), Got: %s\n", (char *)HashTable_get(table, "key1"));

    printf("Removing key2...\n");
    HashTable_remove(table, "key2");
    printf("Expected: (null), Got: %s\n\n", (char *)HashTable_get(table, "key2"));

    HashTable_destroy(&table);
}

void test_resize() {
    printf("Testing hash table resizing...\n");
    HashTable table = HashTable_create(2);
    if (!table) {
        printf("Failed to create hash table.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 20; ++i) {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        HashTable_add(table, key, "value");
    }

    for (int i = 0; i < 20; ++i) {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        const char *value = HashTable_get(table, key);
        if (!value) {
            printf("Failed to retrieve %s after resizing.\n", key);
            exit(EXIT_FAILURE);
        }
    }

    printf("Resize test passed.\n\n");
    HashTable_destroy(&table);
}

int main() {
    printf("Starting HashTable Tests...\n\n");

    test_create_destroy();
    test_add_get();
    test_remove();
    test_resize();

    printf("All tests passed!\n");
    return 0;
}