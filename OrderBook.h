/* OrderBook.h - Header file for the OrderBook module
 *
 * This module manages an entire order book, consisting of a bid side and an ask side.
 * It supports adding and removing orders, retrieving the best bid and ask, getting
 * the top K bids or asks, and returning all executed trades in chronological order.
 *
 * When an order is added, if it crosses (i.e., a buy has a price >= best ask, or a sell
 * has a price <= best bid), trades are executed automatically. The function that adds
 * the order will return an array of trade IDs for the trades that were executed,
 * or NULL if no trades occurred.
 *
 * Author: Your Name
 * Date: January 2025
 */

#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "OrderBookSide.h"

/* Forward declaration of the OrderBook structure. */
typedef struct OrderBook *OrderBook;

/* Trade structure and type definition.
 * A Trade captures a match between a buy order and a sell order.
 */
typedef struct Trade {
    char trade_id[37];        /**< Unique ID for the trade. */
    char buy_order_id[37];    /**< ID of the buy order. */
    char buy_user_id[37];     /**< User ID of the buyer. */
    char sell_order_id[37];   /**< ID of the sell order. */
    char sell_user_id[37];    /**< User ID of the seller. */
    int size;                 /**< Executed trade size (quantity). */
    double price;             /**< Executed trade price. */
    long timestamp;           /**< Timestamp of the trade. */
} *Trade;

/**
 * Creates a new OrderBook instance.
 *
 * @return A newly allocated OrderBook instance, or NULL on failure.
 */
OrderBook OrderBook_create(void);

/**
 * Destroys an OrderBook instance, freeing all associated memory.
 *
 * @param book A pointer to the OrderBook instance to destroy.
 */
void OrderBook_destroy(OrderBook *book);

/**
 * Adds a new order to the OrderBook. If the order crosses with
 * existing orders on the opposite side, trades are executed immediately.
 *
 * @param book The OrderBook instance.
 * @param order The order to add (copied internally).
 * @param trade_count Output pointer for the number of trades executed
 *                    as a result of adding this order. If NULL, NULL is returned.
 * @return An array of trade IDs (dynamically allocated) if trades
 *         occurred, or NULL if no trades were executed.
 *
 * @note If trades are executed, the caller is responsible for freeing
 *       the returned array of trade ID strings. Each string is also
 *       dynamically allocated and must be freed.
 */
char **OrderBook_add_order(OrderBook book, const Order order, int *trade_count);

/**
 * Removes an order from the OrderBook by its ID.
 *
 * @param book The OrderBook instance.
 * @param order_id The unique ID of the order to remove.
 * @return 1 if the order was successfully removed, 0 if not found.
 */
int OrderBook_remove_order(OrderBook book, const char *order_id);

/**
 * Retrieves the current best bid price in the order book.
 *
 * @param book The OrderBook instance.
 * @return The highest bid price, or 0 if no bids exist.
 */
double OrderBook_get_best_bid(OrderBook book);

/**
 * Retrieves the current best ask price in the order book.
 *
 * @param book The OrderBook instance.
 * @return The lowest ask price, or 0 if no asks exist.
 */
double OrderBook_get_best_ask(OrderBook book);

/**
 * Gets the top k bids and top k asks from the order book.
 * If k = 0, returns all available levels on both sides.
 *
 * @param book The OrderBook instance.
 * @param k The number of price levels to retrieve on each side (0 for all).
 * @param bid_levels Output pointer to store array of best bid levels
 *                   (allocated internally).
 * @param bid_count Output pointer for the number of bid levels returned.
 * @param ask_levels Output pointer to store array of best ask levels
 *                   (allocated internally).
 * @param ask_count Output pointer for the number of ask levels returned.
 * @return 1 if successful, 0 on failure.
 *
 * @note The caller is responsible for freeing the returned arrays
 *       (`bid_levels` and `ask_levels`) when no longer needed.
 */
int OrderBook_get_top_levels(OrderBook book,
                             int k,
                             struct OrderBookLevelView **bid_levels,
                             int *bid_count,
                             struct OrderBookLevelView **ask_levels,
                             int *ask_count);

/**
 * Retrieves all executed trades in chronological order.
 *
 * @param book The OrderBook instance.
 * @param trades Output pointer to store the array of executed trades
 *               (allocated internally).
 * @param trade_count Output pointer for the number of executed trades.
 * @return 1 if successful, 0 on failure.
 *
 * @note The caller is responsible for freeing the returned array of
 *       `Trade` objects when no longer needed.
 */
int OrderBook_get_all_trades(OrderBook book, Trade **trades, int *trade_count);

/**
 * Retrieves a single trade from the OrderBook by its trade_id.
 *
 * @param book The OrderBook instance.
 * @param trade_id The unique ID of the trade to retrieve.
 * @return A newly allocated Trade struct if the trade is found, or NULL otherwise.
 *
 * @note The caller is responsible for freeing the returned Trade.
 */
Trade OrderBook_get_trade(OrderBook book, const char *trade_id);

#endif /* ORDER_BOOK_H */