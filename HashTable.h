/* HashTable.h - Header File */
/* Author: Adam Rubinstein
 * Date: 01/18/2025
 * HashTable Module
 * Provides an interface for creating and manipulating a hash table, supporting keys as C strings.
 *
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h> // For size_t

// Opaque structure for the hash table
typedef struct HashTable *HashTable;

/*
 * Function: HashTable_create
 * ---------------------------
 * Creates a new hash table.
 *
 * @param capacity: Initial capacity of the hash table.
 *                  Must be greater than 0.
 * @return: A new HashTable instance, or NULL on failure.
 */
HashTable HashTable_create(size_t capacity);

/*
 * Function: HashTable_destroy
 * ----------------------------
 * Frees all memory associated with the hash table.
 *
 * @param table: A pointer to the Hash table to be destroyed.
 */
void HashTable_destroy(HashTable *table);

/*
 * Function: HashTable_add
 * ------------------------
 * Adds a key-value pair to the hash table. If the key already exists,
 * its value will be updated.
 *
 * @param table: Hash table.
 * @param key: Pointer to the key (null-terminated string).
 * @param value: Pointer to the value to be stored (any type).
 * @return: 0 on success, non-zero on failure.
 */
int HashTable_add(HashTable table, const char *key, void *value);

/*
 * Function: HashTable_get
 * ------------------------
 * Retrieves the value associated with a key.
 *
 * @param table: Hash table.
 * @param key: Pointer to the key (null-terminated string).
 * @return: Pointer to the value if found, NULL if the key does not exist.
 */
void *HashTable_get(const HashTable table, const char *key);

/*
 * Function: HashTable_remove
 * ---------------------------
 * Removes a key-value pair from the hash table.
 *
 * @param table: Hash table.
 * @param key: Pointer to the key (null-terminated string).
 * @return: 0 on success, non-zero if the key does not exist.
 */
int HashTable_remove(HashTable table, const char *key);

#endif // HASHTABLE_H