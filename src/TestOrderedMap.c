/* TestOrderedMap.c - Unit tests for the OrderedMap module
 * 
 * This file contains a main function that rigorously tests the functionality of
 * the OrderedMap module. The tests cover insertion, removal, retrieval, and size
 * operations.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderedMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_test_result(const char *test_name, int result) {
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
}

int main() {
    OrderedMap map = OrderedMap_create();
    if (!map) {
        printf("Failed to create OrderedMap instance\n");
        return 1;
    }

    // Test 1: Insert elements
    int value1 = 42, value2 = 84, value3 = 168;
    print_test_result("Insert 1st element", OrderedMap_insert(map, 10.5, &value1));
    print_test_result("Insert 2nd element", OrderedMap_insert(map, 20.5, &value2));
    print_test_result("Insert 3rd element", OrderedMap_insert(map, 5.5, &value3));

    // Test 2: Retrieve elements
    int *retrieved_value = NULL;
    OrderedMap_get(map, 10.5, (void **)&retrieved_value);
    print_test_result("Retrieve 1st element", retrieved_value && *retrieved_value == 42);

    OrderedMap_get(map, 20.5, (void **)&retrieved_value);
    print_test_result("Retrieve 2nd element", retrieved_value && *retrieved_value == 84);

    OrderedMap_get(map, 5.5, (void **)&retrieved_value);
    print_test_result("Retrieve 3rd element", retrieved_value && *retrieved_value == 168);

    // Test 3: Size of the map
    print_test_result("Map size after inserts", OrderedMap_size(map) == 3);

    // Test 4: Remove elements
    print_test_result("Remove 1st element", OrderedMap_remove(map, 10.5));
    print_test_result("Remove non-existent element", !OrderedMap_remove(map, 15.5));
    print_test_result("Map size after removal", OrderedMap_size(map) == 2);

    // Test 5: Retrieve removed element
    retrieved_value = NULL;
    OrderedMap_get(map, 10.5, (void **)&retrieved_value);
    print_test_result("Retrieve removed element", retrieved_value == NULL);

    // Test 6: Insert 100 random elements
    srand((unsigned)time(NULL));
    int random_values[100];
    for (int i = 0; i < 100; i++) {
        random_values[i] = rand() % 1000;  // Random values between 0 and 999
        OrderedMap_insert(map, (double)i, &random_values[i]);
    }
    print_test_result("Map size after 100 random inserts", OrderedMap_size(map) == 102);

    // Test 7: Verify random elements
    int all_random_elements_correct = 1;
    for (int i = 0; i < 100; i++) {
        int *retrieved;
        OrderedMap_get(map, (double)i, (void **)&retrieved);
        if (!retrieved || *retrieved != random_values[i]) {
            all_random_elements_correct = 0;
            break;
        }
    }
    print_test_result("All random elements correct", all_random_elements_correct);

    // Test 8: Remove 50 random elements
    for (int i = 0; i < 50; i++) {
        OrderedMap_remove(map, (double)i);
    }
    print_test_result("Map size after 50 random removals", OrderedMap_size(map) == 52);

    // Test 9: Verify remaining elements
    int remaining_elements_correct = 1;
    for (int i = 50; i < 100; i++) {
        int *retrieved;
        OrderedMap_get(map, (double)i, (void **)&retrieved);
        if (!retrieved || *retrieved != random_values[i]) {
            remaining_elements_correct = 0;
            break;
        }
    }
    print_test_result("Remaining elements correct", remaining_elements_correct);

    // Test 10: Cleanup
    OrderedMap_destroy(&map);
    printf("All tests completed.\n");

    return 0;
}