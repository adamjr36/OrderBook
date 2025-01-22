#ifndef ORDER_H
#define ORDER_H

// Order structure
typedef struct Order {
    char order_id[37]; /**< Unique ID of the order. */
    char user_id[37];  /**< Unique ID of the user. */
    int quantity;      /**< Quantity of the order. */
    char side;         /**< BUY (1) or SELL (0). */
    double price;      /**< Price of the order. */
    long timestamp;    /**< Timestamp of the order. */
} *Order;

#endif // ORDER_H