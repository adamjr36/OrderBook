/* OrderBookSide.h - Header file for the OrderBookSide module
 * 
 * This module provides the implementation of an OrderBookSide, representing a side
 * (buy or sell) of an order book. It organizes orders into price levels using an
 * OrderedMap and provides functionality to execute trades and manage orders.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#ifndef ORDER_BOOK_SIDE_H
#define ORDER_BOOK_SIDE_H

#include "OrderBookLevel.h"

// OrderBookSide type definition
typedef struct OrderBookSide *OrderBookSide;

struct OrderBookLevelView {
    double price;
    int size;
};

/**
 * Creates a new OrderBookSide instance.
 *
 * @param is_buy_side 1 if this side is for bids, 0 if for asks.
 * @return Newly allocated OrderBookSide instance, or NULL on failure.
 */
OrderBookSide OrderBookSide_create(int is_buy_side);

/**
 * Destroys an OrderBookSide instance, freeing all associated memory.
 *
 * @param side A pointer to the OrderBookSide instance to destroy.
 */
void OrderBookSide_destroy(OrderBookSide *side);

/**
 * Adds an order to the order book side.
 *
 * @param side The OrderBookSide instance.
 * @param order Order to add (copied internally).
 * @return 1 if the operation is successful, 0 on failure.
 */
int OrderBookSide_add_order(OrderBookSide side, const Order order);

/**
 * Gets an order by its ID.
 *
 * @param side The OrderBookSide instance.
 * @param order_id The unique ID of the order to retrieve.
 * @param order Output pointer to store the retrieved order (if found).
 * @return 1 if the order exists, 0 otherwise.
 */
int OrderBookSide_get_order_by_id(OrderBookSide side, const char *order_id, Order *order);

/**
 * Deletes an order by its ID.
 *
 * @param side The OrderBookSide instance.
 * @param order_id The unique ID of the order to delete.
 * @return 1 if the order was successfully deleted, 0 otherwise.
 */
int OrderBookSide_delete_order_by_id(OrderBookSide side, const char *order_id);

/**
 * Executes an incoming order against the most competitive orders on this side.
 *
 * @param side The OrderBookSide instance.
 * @param order The incoming Order to execute (modified in-place).
 * @param filled_orders Output pointer to store an array of filled orders (allocated internally).
 * @param filled_count Output pointer to store the count of filled orders.
 * @return 1 if the operation is successful, 0 on failure.
 */
int OrderBookSide_execute_against(OrderBookSide side, Order order, Order **filled_orders, int *filled_count);

/**
 * Gets the best price level on this side of the order book.
 *
 * @param side The OrderBookSide instance.
 * @return The best price (highest for buy side, lowest for sell side), or 0 if no levels exist.
 */
double OrderBookSide_get_best_price(OrderBookSide side);

/**
 * Gets the k most competitive price levels on this side of the order book. If k is 0 then all levels are returned.
 * 
 * @param side The OrderBookSide instance.
 * @param k The number of levels to return, or 0 for all levels.
 * @param levels Output pointer to store the array of levels (allocated internally).
 * @param level_count Output pointer to store the count of levels.
 * @return 1 if the operation is successful, 0 on failure.
 */
int OrderBookSide_get_levels(OrderBookSide side, int k, struct OrderBookLevelView **levels, int *level_count);

#endif // ORDER_BOOK_SIDE_H
