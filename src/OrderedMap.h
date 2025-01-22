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

typedef struct OrderedMapIterator *OrderedMapIterator;

/**
 * Creates a new OrderedMap instance.
 *
 * @return A newly allocated OrderedMap instance, or NULL on failure.
 */
OrderedMap OrderedMap_create();

/**
 * Destroys an OrderedMap instance, freeing all associated memory.
 *
 * @param map A pointer to the OrderedMap instance to destroy.
 */
void OrderedMap_destroy(OrderedMap *map);

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

/**
 * Creates a new iterator for the map, starting at the first key-value pair.
 *
 * @param map The OrderedMap instance.
 * @return A newly allocated OrderedMapIterator instance, or NULL on failure.
 */
OrderedMapIterator OrderedMap_front(OrderedMap map);

/**
 * Creates a new iterator for the map, starting at the last key-value pair.
 *
 * @param map The OrderedMap instance.
 * @return A newly allocated OrderedMapIterator instance, or NULL on failure.
 */
OrderedMapIterator OrderedMap_back(OrderedMap map);

/**
 * Destroys an OrderedMapIterator instance, freeing all associated memory.
 *
 * @param iter A pointer to the OrderedMapIterator instance to destroy.
 */
void OrderedMapIterator_destroy(OrderedMapIterator *iter);

/**
 * Retrieves the key-value pair at the current iterator position.
 *
 * @param iter The OrderedMapIterator instance.
 * @param key Output pointer to store the key.
 * @param value Output pointer to store the value.
 * @return 1 if the iterator is valid, 0 if the end of the map is reached.
 */
int OrderedMapIterator_get(OrderedMapIterator iter, double *key, void **value);

/**
 * Advances the iterator to the next key-value pair in the map.
 *
 * @param iter The OrderedMapIterator instance.
 * @return 1 if the iterator was successfully advanced, 0 if the end of the map is reached.
 */
int OrderedMapIterator_next(OrderedMapIterator iter);

/**
 * Advances the iterator to the previous key-value pair in the map.
 *
 * @param iter The OrderedMapIterator instance.
 * @return 1 if the iterator was successfully advanced, 0 if the beginning of the map is reached.
 */
int OrderedMapIterator_prev(OrderedMapIterator iter);

#endif // ORDERED_MAP_H