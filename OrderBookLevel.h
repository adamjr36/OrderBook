/* OrderBookLevel.h - Header file for the OrderBookLevel module
 * 
 * This module provides the implementation of an OrderBookLevel, which represents
 * a price level in an order book. A price level represents one or more bid or ask orders at the same price.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */
#ifndef ORDER_BOOK_LEVEL_H
#define ORDER_BOOK_LEVEL_H

#include <stddef.h>
#include <stdbool.h>

// Order structure
typedef struct Order {
    char order_id[37]; /**< Unique ID of the order. */
    char user_id[37];  /**< Unique ID of the user. */
    int quantity;      /**< Quantity of the order. */
    char side;         /**< BUY (1) or SELL (0). */
    double price;      /**< Price of the order. */
    long timestamp;    /**< Timestamp of the order. */
} *Order;

// OrderBookLevel type definition
typedef struct OrderBookLevel *OrderBookLevel;

/**
 * Creates a new OrderBookLevel instance.
 *
 * @param price The price of the level.
 * @return A newly allocated OrderBookLevel instance, or NULL on failure.
 */
OrderBookLevel OrderBookLevel_create(double price);

/**
 * Destroys an OrderBookLevel instance, freeing all associated memory.
 *
 * @param level A pointer to the OrderBookLevel instance to destroy.
 */
void OrderBookLevel_destroy(OrderBookLevel *level);

/**
 * Adds an order to the level.
 *
 * @param level The OrderBookLevel instance.
 * @param order The Order to add (copied internally).
 * @return 1 if the operation is successful, 0 on failure.
 */
int OrderBookLevel_add_order(OrderBookLevel level, const Order order);

/**
 * Gets the oldest order from the level.
 *
 * @param level The OrderBookLevel instance.
 * @param order Output pointer to store the retrieved order (if not NULL).
 * @return 1 if an order was retrieved, 0 if the level is empty.
 */
int OrderBookLevel_get_order(OrderBookLevel level, Order *order);

/**
 * Gets an order by its order_id.
 *
 * @param level The OrderBookLevel instance.
 * @param order_id The unique ID of the order to retrieve.
 * @param order Output pointer to store the retrieved order (if not NULL).
 * @return 1 if the order was found, 0 otherwise.
 */
int OrderBookLevel_get_order_by_id(OrderBookLevel level, const char *order_id, Order *order);

/**
 * Removes the oldest order from the level.
 *
 * @param level The OrderBookLevel instance.
 * @param order Output pointer to store the removed order (if not NULL).
 * @return 1 if an order was removed, 0 if the level is empty.
 */
int OrderBookLevel_remove_order(OrderBookLevel level, Order order);

/**
 * Removes an order by its order_id.
 * 
 * @param level The OrderBookLevel instance.
 * @param order_id The unique ID of the order to remove.
 * @return 1 if the order was successfully removed, 0 otherwise.
 */
int OrderBookLevel_delete_order_by_id(OrderBookLevel level, const char *order_id);

/**
 * Gets the total quantity of all orders at this price level.
 *
 * @param level The OrderBookLevel instance.
 * @return The total quantity of orders at this price level.
 */
int OrderBookLevel_get_total_quantity(const OrderBookLevel level);

/**
 * Recalculates the total quantity of the level.
 *
 * @param level The OrderBookLevel instance.
 */
void OrderBookLevel_reset_total_quantity(OrderBookLevel level);

/**
 * Checks if the level is empty.
 *
 * @param level The OrderBookLevel instance.
 * @return true if the level is empty, false otherwise.
 */
bool OrderBookLevel_is_empty(const OrderBookLevel level);

#endif // ORDER_BOOK_LEVEL_H