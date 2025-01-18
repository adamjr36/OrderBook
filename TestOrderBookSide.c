/*******************************************************************************************/

/* TestOrderBookSide.c - Test file for the OrderBookSide module
 *
 * This file rigorously tests the OrderBookSide module, including adding, retrieving,
 * deleting, and executing orders.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderBookSide.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create an order
struct Order create_order(const char *order_id, const char *user_id, int quantity, char side, double price, long timestamp) {
    struct Order order;
    strncpy(order.order_id, order_id, sizeof(order.order_id) - 1);
    strncpy(order.user_id, user_id, sizeof(order.user_id) - 1);
    order.quantity = quantity;
    order.side = side;
    order.price = price;
    order.timestamp = timestamp;
    return order;
}

// Function to print an order
void print_order(const Order order) {
    printf("Order ID: %s, User ID: %s, Quantity: %d, Side: %c, Price: %.2f, Timestamp: %ld\n",
           order->order_id, order->user_id, order->quantity, order->side, order->price, order->timestamp);
}

// Function to check test results
void log_test_result(const char *test_name, int condition, const char *expected, double actual) {
    if (condition) {
        printf("%s: PASSED\n", test_name);
    } else {
        printf("%s: FAILED\nExpected: %s\nActual: %f\n", test_name, expected, actual);
    }
}

// Main function to test OrderBookSide
int main() {
    printf("Testing OrderBookSide Module\n\n");

    // Create an OrderBookSide for the sell side
    OrderBookSide sell_side = OrderBookSide_create(0);
    if (!sell_side) {
        printf("Failed to create OrderBookSide\n");
        return 1;
    }

    // Add orders
    printf("Adding orders...\n");
    struct Order order1 = create_order("order1", "user1", 10, 'S', 100.0, 1);
    struct Order order2 = create_order("order2", "user2", 15, 'S', 105.0, 2);
    struct Order order3 = create_order("order3", "user3", 20, 'S', 100.0, 3);
    OrderBookSide_add_order(sell_side, &order1);
    OrderBookSide_add_order(sell_side, &order2);
    OrderBookSide_add_order(sell_side, &order3);

    // Test get_best_price
    double best_price = OrderBookSide_get_best_price(sell_side);
    log_test_result("Test get_best_price", best_price == 100.0, "100.0", best_price);

    // Test get_order_by_id
    printf("Retrieving order by ID: order1\n");
    Order retrieved_order = NULL;
    if (OrderBookSide_get_order_by_id(sell_side, "order1", &retrieved_order)) {
        log_test_result("Test get_order_by_id", retrieved_order->quantity == 10, "10", retrieved_order->quantity);
    } else {
        printf("Test get_order_by_id: FAILED\nExpected: order found\nActual: order not found\n");
    }

    // Test delete_order_by_id
    printf("Deleting order by ID: order1\n");
    int delete_result = OrderBookSide_delete_order_by_id(sell_side, "order1");
    log_test_result("Test delete_order_by_id", delete_result == 1, "1", delete_result);

    // Test execute_against
    printf("Executing against sell side...\n");
    struct Order incoming_order = create_order("incoming", "user4", 25, 'B', 105.0, 4);
    Order *filled_orders = NULL;
    int filled_count = 0;
    if (OrderBookSide_execute_against(sell_side, &incoming_order, &filled_orders, &filled_count)) {
        printf("Filled orders:\n");
        for (int i = 0; i < filled_count; ++i) {
            print_order(filled_orders[i]);
        }
        free(filled_orders);
    } else {
        printf("Test execute_against: FAILED\nExpected: successful execution\nActual: execution failed\n");
    }

    // Test best price after execution
    best_price = OrderBookSide_get_best_price(sell_side);
    log_test_result("Test best_price_after_execution", best_price == 105.0, "105.0", best_price);

    // Cleanup
    OrderBookSide_destroy(sell_side);
    printf("Testing completed\n");

    return 0;
}
