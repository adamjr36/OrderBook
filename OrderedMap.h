/* OrderedMap.h - Header file for the OrderedMap module
 * Author: Adam Rubinstein
 * Date: 01/18/2025
 * 
 * This module provides an ordered map data structure, supporting keys as doubles. 
 * An OrderedMap stores key-value pairs, allowing for efficient insertion, deletion,
 * and lookup operations while maintaining keys in sorted order.
 * All Operations are performed in O(log n) time complexity.
 * 
 */
#ifndef ORDERED_MAP_H
#define ORDERED_MAP_H

#include <stddef.h>

// Type definitions
typedef struct OrderedMap *OrderedMap;

// Function contracts

/**
 * Creates a new OrderedMap instance.
 *
 * @return A newly allocated OrderedMap instance, or NULL on failure.
 */
OrderedMap OrderedMap_create();

/**
 * Destroys an OrderedMap instance, freeing all associated memory.
 *
 * @param map The OrderedMap instance to destroy.
 */
void OrderedMap_destroy(OrderedMap map);

/**
 * Inserts a key-value pair into the map. If the key already exists, updates its value.
 *
 * @param map The OrderedMap instance.
 * @param key The key to insert (must be unique in the map).
 * @param value The value to associate with the key.
 * @return 1 if the operation is successful, 0 on failure.
 */
int OrderedMap_insert(OrderedMap map, double key, void *value);

/**
 * Removes a key-value pair from the map by key.
 *
 * @param map The OrderedMap instance.
 * @param key The key of the pair to remove.
 * @return 1 if the key was found and removed, 0 if the key does not exist.
 */
int OrderedMap_remove(OrderedMap map, double key);

/**
 * Retrieves the value associated with a key.
 *
 * @param map The OrderedMap instance.
 * @param key The key to look up.
 * @param value Output pointer to store the associated value if found.
 * @return 1 if the key exists, 0 otherwise.
 */
int OrderedMap_get(const OrderedMap map, double key, void **value);

/**
 * Retrieves the key-value pair with the minimum key in the map.
 *
 * @param map The OrderedMap instance.
 * @param key Output pointer to store the minimum key.
 * @param value Output pointer to store the value associated with the minimum key.
 * @return 1 if the map is not empty, 0 if the map is empty.
 */
int OrderedMap_get_min(const OrderedMap map, double *key, void **value);

/**
 * Retrieves the key-value pair with the maximum key in the map.
 *
 * @param map The OrderedMap instance.
 * @param key Output pointer to store the maximum key.
 * @param value Output pointer to store the value associated with the maximum key.
 * @return 1 if the map is not empty, 0 if the map is empty.
 */
int OrderedMap_get_max(const OrderedMap map, double *key, void **value);

/**
 * Gets the number of key-value pairs in the map.
 *
 * @param map The OrderedMap instance.
 * @return The number of key-value pairs in the map.
 */
size_t OrderedMap_size(const OrderedMap map);

#endif // ORDERED_MAP_H