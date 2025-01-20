/* TestOrderBookLevel.c - Unit tests for the OrderBookLevel module
 * 
 * This file contains a main function that rigorously tests the functionality of
 * the OrderBookLevel module. The tests cover adding, removing, retrieving, and
 * checking total quantities of orders.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderBookLevel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_test_result(const char *test_name, int result) {
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
}

int main() {
    OrderBookLevel level = OrderBookLevel_create(100.50);
    if (!level) {
        printf("Failed to create OrderBookLevel instance\n");
        return 1;
    }

    // Test 1: Add orders
    struct Order order1 = {"order1", "user1", 10, 'B', 100.50, 1622515800};
    struct Order order2 = {"order2", "user2", 20, 'B', 100.50, 1622515900};
    struct Order order3 = {"order3", "user3", 30, 'B', 100.50, 1622516000};

    print_test_result("Add 1st order", OrderBookLevel_add_order(level, &order1));
    print_test_result("Add 2nd order", OrderBookLevel_add_order(level, &order2));
    print_test_result("Add 3rd order", OrderBookLevel_add_order(level, &order3));

    // Test 2: Total quantity
    print_test_result("Total quantity check", OrderBookLevel_get_total_quantity(level) == 60);

    // Test 3: Remove orders
    struct Order ro = {0};
    Order removed_order = &ro;
    print_test_result("Remove 1st order", OrderBookLevel_remove_order(level, removed_order) && strcmp(removed_order->order_id, "order1") == 0);
    print_test_result("Total quantity after 1st removal", OrderBookLevel_get_total_quantity(level) == 50);

    print_test_result("Remove 2nd order", OrderBookLevel_remove_order(level, removed_order) && strcmp(removed_order->order_id, "order2") == 0);
    print_test_result("Total quantity after 2nd removal", OrderBookLevel_get_total_quantity(level) == 30);

    print_test_result("Remove 3rd order", OrderBookLevel_remove_order(level, removed_order) && strcmp(removed_order->order_id, "order3") == 0);
    print_test_result("Total quantity after 3rd removal", OrderBookLevel_get_total_quantity(level) == 0);

    // Test 4: Remove from empty level
    print_test_result("Remove from empty level", !OrderBookLevel_remove_order(level, NULL));

    // Test 5: Is empty check
    print_test_result("Level is empty", OrderBookLevel_is_empty(level));

    // Test 6: Add and remove many orders
    int passed = 1;
    for (int i = 0; i < 100; i++) {
        struct Order order = {"", "user", i + 1, 'B', 100.50, 1622516000 + i};
        snprintf(order.order_id, sizeof(order.order_id), "order%d", i + 1);
        passed &= OrderBookLevel_add_order(level, &order);
    }
    print_test_result("Add 100 orders", passed);
    print_test_result("Total quantity after adding 100 orders", OrderBookLevel_get_total_quantity(level) == 5050);

    passed = 1;
    for (int i = 0; i < 100; i++) {
        passed &= OrderBookLevel_remove_order(level, removed_order);
    }
    print_test_result("Remove 100 orders", passed);
    print_test_result("Level is empty after removing 100 orders", OrderBookLevel_is_empty(level));

    // Cleanup
    OrderBookLevel_destroy(&level);
    printf("All tests completed.\n");

    return 0;
}