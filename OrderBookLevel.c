/*******************************************************************************************/
/* OrderBookLevel.c - Implementation file for the OrderBookLevel module
 *
 * This file implements an OrderBookLevel, which represents a price level in an order book.
 * A level maintains a queue of orders with the same price.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderBookLevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Order node structure for the queue
typedef struct OrderNode {
    struct Order order;
    struct OrderNode *next;
} *OrderNode;

struct OrderBookLevel {
    double price;          /**< Price of the level. */
    OrderNode head;       /**< Head of the order queue. */
    OrderNode tail;       /**< Tail of the order queue. */
    int total_quantity;    /**< Total quantity of orders at this level. */
};

// Helper function prototypes
static OrderNode create_order_node(const Order order);
static void destroy_order_node(OrderNode node);

// Public function implementations
OrderBookLevel OrderBookLevel_create(double price) {
    OrderBookLevel level = malloc(sizeof(struct OrderBookLevel));
    if (!level) return NULL;
    level->price = price;
    level->head = NULL;
    level->tail = NULL;
    level->total_quantity = 0;
    return level;
}

void OrderBookLevel_destroy(OrderBookLevel level) {
    if (!level) return;
    OrderNode current = level->head;
    while (current) {
        OrderNode next = current->next;
        destroy_order_node(current);
        current = next;
    }
    free(level);
}

int OrderBookLevel_add_order(OrderBookLevel level, const Order order) {
    if (!level || !order) return 0;
    OrderNode node = create_order_node(order);
    if (!node) return 0;

    if (!level->head) {
        level->head = level->tail = node;
    } else {
        level->tail->next = node;
        level->tail = node;
    }

    level->total_quantity += order->quantity;
    return 1;
}

// Returns a pointer to the actual Order in the level, not a copy
int OrderBookLevel_get_order(OrderBookLevel level, Order *order) {
    if (!level || !level->head) return 0;

    if (order) *order = &level->head->order;
    return 1;
}

int OrderBookLevel_get_order_by_id(OrderBookLevel level, const char *order_id, Order *order) {
    if (!level || !order_id || !level->head) return 0;

    OrderNode current = level->head;
    while (current) {
        if (strcmp(current->order.order_id, order_id) == 0) {
            if (order) *order = &current->order;
            return 1;
        }
        current = current->next;
    }

    return 0;
}

int OrderBookLevel_remove_order(OrderBookLevel level, Order order) {
    if (!level || !level->head) return 0;

    OrderNode node = level->head;
    if (order) memcpy(order, &node->order, sizeof(struct Order));

    level->head = node->next;
    if (!level->head) level->tail = NULL;

    level->total_quantity -= node->order.quantity;
    destroy_order_node(node);

    return 1;
}

int OrderBookLevel_delete_order_by_id(OrderBookLevel level, const char *order_id) {
    if (!level || !order_id || !level->head) return 0;

    OrderNode prev = NULL;
    OrderNode current = level->head;
    while (current) {
        if (strcmp(current->order.order_id, order_id) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                level->head = current->next;
            }

            if (current == level->tail) {
                level->tail = prev;
            }

            level->total_quantity -= current->order.quantity;
            destroy_order_node(current);
            return 1;
        }

        prev = current;
        current = current->next;
    }

    return 0;
}

int OrderBookLevel_get_total_quantity(const OrderBookLevel level) {
    return level ? level->total_quantity : 0;
}

bool OrderBookLevel_is_empty(const OrderBookLevel level) {
    return level ? level->head == NULL : true;
}

// Helper function implementations
static OrderNode create_order_node(const Order order) {
    OrderNode node = malloc(sizeof(struct OrderNode));
    if (!node) return NULL;
    memcpy(&node->order, order, sizeof(struct Order));
    node->next = NULL;
    return node;
}

static void destroy_order_node(OrderNode node) {
    if (node) free(node);
}