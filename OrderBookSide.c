/*******************************************************************************************/

/* OrderBookSide.c - Implementation file for the OrderBookSide module
 *
 * This file implements an OrderBookSide, which represents one side (buy or sell) of an
 * order book. Orders are organized into price levels, and trades are executed by matching
 * orders with the most competitive prices.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderBookSide.h"
#include "HashTable.h"
#include "OrderedMap.h"
#include "OrderBookLevel.h"
#include <stdlib.h>
#include <string.h>

struct OrderBookSide {
    OrderedMap levels; /**< OrderedMap of price levels (price -> OrderBookLevel). */
    HashTable order_lookup; /**< Hashtable for order ID lookup. */
    int is_buy_side; /**< 1 if this is the buy side, 0 if the sell side. */
};

// Helper function prototypes
static int compare_prices(double price1, double price2, int is_buy_side);

// Public function implementations
OrderBookSide OrderBookSide_create(int is_buy_side) {
    OrderBookSide side = malloc(sizeof(struct OrderBookSide));
    if (!side) return NULL;
    side->levels = OrderedMap_create();
    if (!side->levels) {
        free(side);
        return NULL;
    }
    side->order_lookup = HashTable_create(1024); // Added default capacity
    if (!side->order_lookup) {
        OrderedMap_destroy(side->levels);
        free(side);
        return NULL;
    }
    side->is_buy_side = is_buy_side;
    return side;
}

void OrderBookSide_destroy(OrderBookSide side) {
    if (!side) return;

    OrderedMap_destroy(side->levels);
    HashTable_destroy(side->order_lookup);
    free(side);
}

int OrderBookSide_add_order(OrderBookSide side, const Order order) {
    if (!side || !order) return 0;

    OrderBookLevel level = NULL;
    if (!OrderedMap_get(side->levels, order->price, (void **)&level)) {
        level = OrderBookLevel_create(order->price);
        if (!level) return 0;
        OrderedMap_insert(side->levels, order->price, level);
    }

    if (!OrderBookLevel_add_order(level, order)) return 0;

    HashTable_add(side->order_lookup, order->order_id, level);
    return 1;
}

int OrderBookSide_get_order_by_id(OrderBookSide side, const char *order_id, Order *order) {
    if (!side || !order_id) return 0;

    OrderBookLevel level = NULL;
    if (!(level = HashTable_get(side->order_lookup, order_id))) return 0;

    return OrderBookLevel_get_order_by_id(level, order_id, order);
}

int OrderBookSide_delete_order_by_id(OrderBookSide side, const char *order_id) {
    if (!side || !order_id) return 0;

    OrderBookLevel level = NULL;
    if (!(level = HashTable_get(side->order_lookup, order_id))) return 0;

    if (!OrderBookLevel_delete_order_by_id(level, order_id)) return 0;

    HashTable_remove(side->order_lookup, order_id);
    return 1;
}

int OrderBookSide_execute_against(OrderBookSide side, Order order, Order **filled_orders, int *filled_count) {
    if (!side || !order || !filled_orders || !filled_count) return 0;

    *filled_orders = NULL;
    *filled_count = 0;

    double price;
    OrderBookLevel level;

    while (order->quantity > 0 && OrderedMap_size(side->levels) > 0) {
        if (side->is_buy_side) {
            OrderedMap_get_max(side->levels, &price, (void **)&level);
        } else {
            OrderedMap_get_min(side->levels, &price, (void **)&level);
        }

        if (!compare_prices(price, order->price, side->is_buy_side)) break;

        while (order->quantity > 0 && !OrderBookLevel_is_empty(level)) {
            Order other;
            OrderBookLevel_get_order(level, &other);

            int filled_quantity = other->quantity < order->quantity ? other->quantity : order->quantity;
            order->quantity -= filled_quantity;

            Order filled_order = malloc(sizeof(struct Order));
            // If the filled order is completely filled, remove it from the level
            if (other->quantity == filled_quantity) {
                OrderBookLevel_remove_order(level, filled_order);
            // Otherwise, update the filled order's quantity
            } else {
                other->quantity = filled_quantity;
                memcpy(filled_order, other, sizeof(struct Order));
            }

            *filled_orders = realloc(*filled_orders, (*filled_count + 1) * sizeof(Order)); // vec?
            (*filled_orders)[*filled_count] = filled_order;
            (*filled_count)++;
        }

        if (OrderBookLevel_is_empty(level)) {
            OrderedMap_remove(side->levels, price);
            OrderBookLevel_destroy(level);
        }
    }

    return 1;
}

double OrderBookSide_get_best_price(OrderBookSide side) {
    if (!side || OrderedMap_size(side->levels) == 0) return 0.0;

    double best_price;
    if (side->is_buy_side) {
        OrderedMap_get_max(side->levels, &best_price, NULL);
    } else {
        OrderedMap_get_min(side->levels, &best_price, NULL);
    }

    return best_price;
}

// Helper function implementations
static int compare_prices(double price1, double price2, int is_buy_side) {
    return is_buy_side ? price1 >= price2 : price1 <= price2;
}
