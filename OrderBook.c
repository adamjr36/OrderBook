/* OrderBook.c - Implementation of the OrderBook module
 *
 * This module manages an entire order book, consisting of a bid side and an ask side.
 * It supports adding and removing orders, retrieving the best bid and ask, getting
 * the top K bids or asks, and returning all executed trades in chronological order.
 *
 * When an order is added, if it crosses with existing orders on the opposite side
 * (i.e., a buy order with price >= best ask or a sell order with price <= best bid),
 * trades are executed automatically. The function that adds the order returns an
 * array of trade IDs for any executed trades, or NULL if no trades occurred.
 *
 * Author: Your Name
 * Date: January 2025
 */

#include "OrderBook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Private struct for the OrderBook */
struct OrderBook {
    OrderBookSide bid_side;        /* The buy side of the order book */
    OrderBookSide ask_side;        /* The sell side of the order book */

    Trade *executed_trades;        /* Dynamic array of executed trades */
    int trade_count;               /* Number of executed trades stored */
    int trade_capacity;            /* Current capacity of executed_trades array */
};

/* Forward declarations of internal (static) helper functions. */
static void   generate_trade_id(char *buf, size_t len);
static Trade  create_trade(const Order incoming, const Order matched);
static void   add_trade_to_book(OrderBook book, const Trade t);
static time_t get_current_timestamp(void);

/*
 * OrderBook_create
 * ----------------
 * Allocates and initializes a new OrderBook instance.
 */
OrderBook OrderBook_create(void)
{
    OrderBook book = (OrderBook)malloc(sizeof(*book));
    if (!book) {
        return NULL;
    }

    /* Create bid and ask sides */
    book->bid_side = OrderBookSide_create(/* is_buy_side = */ 1);
    book->ask_side = OrderBookSide_create(/* is_buy_side = */ 0);

    if (!book->bid_side || !book->ask_side) {
        /* Cleanup if side creation fails */
        if (book->bid_side) OrderBookSide_destroy(&(book->bid_side));
        if (book->ask_side) OrderBookSide_destroy(&(book->ask_side));
        free(book);
        return NULL;
    }

    book->executed_trades = NULL;
    book->trade_count = 0;
    book->trade_capacity = 0;

    return book;
}

/*
 * OrderBook_destroy
 * -----------------
 * Frees the memory associated with an OrderBook instance,
 * including its sides and all stored trades.
 */
void OrderBook_destroy(OrderBook *book) {
    if (!book || !*book) return;
    OrderBook b = *book;

    /* Destroy the sides */
    OrderBookSide_destroy(&(b->bid_side));
    OrderBookSide_destroy(&(b->ask_side));
    /* Free executed trades */
    if (b->executed_trades) {
        /* Each Trade is allocated as part of the array; freeing the array is sufficient */
        for (int i = 0; i < b->trade_count; i++) {
            free(b->executed_trades[i]);
        }
        free(b->executed_trades);
    }

    /* Free the OrderBook itself */
    free(b);
    *book = NULL;
}

/*
 * OrderBook_add_order
 * -------------------
 * Adds a new order to the order book. If the order crosses,
 * it will execute against the opposite side. Returns an array of trade IDs
 * for any executed trades or NULL if no trades occurred.
 *
 * The caller must free each returned string, then free the array itself.
 */
char **OrderBook_add_order(OrderBook book, const Order order, int *trade_count)
{
    if (!book || !order) {
        return NULL;
    }

    /* Copy the incoming order so we can modify its quantity if partially filled. */
    Order incoming = (Order)malloc(sizeof(*incoming));
    if (!incoming) {
        return NULL;
    }
    memcpy(incoming, order, sizeof(*incoming));

    /* Determine if this is a buy ('1') or sell ('0'). */
    int is_buy = (incoming->side == '1');

    /* We'll store the filled orders from the opposite side. */
    Order *filled_orders = NULL;
    int filled_count = 0;

    /*
     * Execute against the opposite side if crossing can occur.
     * - If buy: execute against ask side.
     * - If sell: execute against bid side.
     */
    if (is_buy) {
        /* Attempt to fill this incoming buy order against the ask side */
        if (!OrderBookSide_execute_against(book->ask_side, incoming, &filled_orders, &filled_count)) {
            /* Some error occurred in execution, just clean up and return. */
            free(incoming);
            return NULL;
        }
    } else {
        /* Sell order executes against the bid side */
        if (!OrderBookSide_execute_against(book->bid_side, incoming, &filled_orders, &filled_count)) {
            /* Some error occurred */
            free(incoming);
            return NULL;
        }
    }

    /*
     * For every filled order, we create a Trade record.
     * Then we add it to the book's executed trades array.
     * We'll also build an array of trade IDs to return.
     */
    char **trade_ids = NULL;
    int num_trades_executed = 0;
    if (filled_count > 0) {
        trade_ids = (char **)malloc(sizeof(char *) * filled_count);
        if (!trade_ids) {
            /* Cleanup on failure */
            free(incoming);
            free(filled_orders);
            return NULL;
        }
        memset(trade_ids, 0, sizeof(char *) * filled_count);

        for (int i = 0; i < filled_count; i++) {
            /* Create a Trade from the perspective of (incoming, filled_orders[i]) */
            Trade t = create_trade(incoming, filled_orders[i]);

            /* Add the trade to the order book's record */
            add_trade_to_book(book, t);

            /* Copy out the trade_id so we can return it to the caller */
            trade_ids[i] = (char *)malloc(37);
            if (trade_ids[i]) {
                strncpy(trade_ids[i], t->trade_id, 37);
            }
            num_trades_executed++;
            free(filled_orders[i]);
        }

        free(filled_orders);
    }

    /*
     * If there's still quantity left in the incoming order (partial fill),
     * place the remainder on the correct side of the book.
     */
    if (incoming->quantity > 0) {
        if (is_buy) {
            /* Add to bid side */
            OrderBookSide_add_order(book->bid_side, incoming);
        } else {
            /* Add to ask side */
            OrderBookSide_add_order(book->ask_side, incoming);
        }
    }

    free(incoming);

    if (trade_count) *trade_count = num_trades_executed;

    /* If no trades were executed, free the trade_ids array (if allocated) and return NULL. */
    if (num_trades_executed == 0 || !trade_count) {
        if (trade_ids) {
            for (int i = 0; i < filled_count; i++) {
                free(trade_ids[i]);
            }
            free(trade_ids);
        }
        return NULL;
    }

    return trade_ids; /* Caller must free each string and then the array. */
}

/*
 * OrderBook_remove_order
 * ----------------------
 * Removes an order from either the bid side or ask side by its ID.
 * Returns 1 if successful, 0 if not found.
 */
int OrderBook_remove_order(OrderBook book, const char *order_id)
{
    if (!book || !order_id) {
        return 0;
    }

    /*
     * Try removing from the bid side first.
     * If not found, try removing from the ask side.
     */
    int removed = OrderBookSide_delete_order_by_id(book->bid_side, order_id);
    if (!removed) {
        removed = OrderBookSide_delete_order_by_id(book->ask_side, order_id);
    }

    return removed;
}

/*
 * OrderBook_get_best_bid
 * ----------------------
 * Retrieves the highest bid price in the order book, or 0 if no bids.
 */
double OrderBook_get_best_bid(OrderBook book)
{
    if (!book) {
        return 0.0;
    }
    return OrderBookSide_get_best_price(book->bid_side);
}

/*
 * OrderBook_get_best_ask
 * ----------------------
 * Retrieves the lowest ask price in the order book, or 0 if no asks.
 */
double OrderBook_get_best_ask(OrderBook book)
{
    if (!book) {
        return 0.0;
    }
    return OrderBookSide_get_best_price(book->ask_side);
}

/*
 * OrderBook_get_top_levels
 * ------------------------
 * Retrieves the top k levels (or all if k=0) for both the bid and ask sides.
 * The caller is responsible for freeing the returned arrays.
 */
int OrderBook_get_top_levels(OrderBook book,
                             int k,
                             struct OrderBookLevelView **bid_levels,
                             int *bid_count,
                             struct OrderBookLevelView **ask_levels,
                             int *ask_count)
{
    if (!book || !bid_levels || !bid_count || !ask_levels || !ask_count) {
        return 0;
    }

    if (!OrderBookSide_get_levels(book->bid_side, k, bid_levels, bid_count)) {
        return 0;
    }
    if (!OrderBookSide_get_levels(book->ask_side, k, ask_levels, ask_count)) {
        /* Clean up the bid_levels in case the ask_side call fails. */
        free(*bid_levels);
        *bid_levels = NULL;
        *bid_count = 0;
        return 0;
    }

    return 1;
}

/*
 * OrderBook_get_all_trades
 * -------------------------
 * Returns all executed trades in chronological order (the order in which
 * they were added to the order book). The caller is responsible for freeing
 * the returned array of Trade objects.
 */
int OrderBook_get_all_trades(OrderBook book, Trade **trades, int *trade_count)
{
    if (!book || !trades || !trade_count) {
        return 0;
    }

    if (book->trade_count == 0) {
        *trades = NULL;
        *trade_count = 0;
        return 1;
    }

    /* Allocate a new array of Trade pointers and copy them over. */
    *trades = (Trade *)malloc(book->trade_count * sizeof(Trade));
    if (!*trades) {
        return 0;
    }

    for (int i = 0; i < book->trade_count; i++) {
        /* Allocate a new Trade struct and copy the fields. */
        Trade new_t = (Trade)malloc(sizeof(*(book->executed_trades[i])));
        if (!new_t) {
            /* In case of partial allocation failure, free everything so far */
            for (int j = 0; j < i; j++) {
                free((*trades)[j]);
            }
            free(*trades);
            *trades = NULL;
            *trade_count = 0;
            return 0;
        }
        memcpy(new_t, book->executed_trades[i], sizeof(*(book->executed_trades[i])));
        (*trades)[i] = new_t;
    }

    *trade_count = book->trade_count;
    return 1;
}

/*
 * OrderBook_get_trade
 * -------------------
 * Searches executed trades by trade_id. Returns a heap-allocated copy
 * of the matching Trade, or NULL if not found.
 */
Trade OrderBook_get_trade(OrderBook book, const char *trade_id)
{
    if (!book || !trade_id) {
        return NULL;
    }

    for (int i = 0; i < book->trade_count; i++) {
        /* Compare trade_id with each recorded trade's ID */
        if (strncmp(book->executed_trades[i]->trade_id, trade_id, sizeof(book->executed_trades[i]->trade_id)) == 0) {
            /* Found a match, create a copy to return */
            Trade copy = (Trade)malloc(sizeof(*copy));
            if (!copy) {
                return NULL; /* Allocation failure */
            }
            memcpy(copy, book->executed_trades[i], sizeof(*copy));
            return copy;
        }
    }

    /* No match found */
    return NULL;
} 

/* ======================= */
/* Internal Helper Methods */
/* ======================= */

/*
 * generate_trade_id
 * -----------------
 * Generates a pseudo-unique trade ID as a string (36 chars + null terminator).
 * In production, replace with a robust UUID generator.
 */
static void generate_trade_id(char *buf, size_t len)
{
    static int counter = 0;
    snprintf(buf, len, "TRADE-%08d", counter++);
}

/*
 * create_trade
 * ------------
 * Constructs a Trade from an incoming order and a matched order. The
 * buy order must have side '1', the sell order '0'. If the roles are flipped,
 * this function deduces them automatically.
 */
static Trade create_trade(const Order incoming, const Order matched)
{
    Trade t = (Trade)malloc(sizeof(*t));
    if (!t) {
        return NULL;
    }

    memset(t, 0, sizeof(*t));
    generate_trade_id(t->trade_id, sizeof(t->trade_id));

    /* If 'incoming' is the buyer, fill buyer fields from incoming, else from matched. */
    int incoming_is_buy = (incoming->side == '1');

    if (incoming_is_buy) {
        strncpy(t->buy_order_id, incoming->order_id, sizeof(t->buy_order_id));
        strncpy(t->buy_user_id, incoming->user_id, sizeof(t->buy_user_id));
        strncpy(t->sell_order_id, matched->order_id, sizeof(t->sell_order_id));
        strncpy(t->sell_user_id, matched->user_id, sizeof(t->sell_user_id));
    } else {
        strncpy(t->buy_order_id, matched->order_id, sizeof(t->buy_order_id));
        strncpy(t->buy_user_id, matched->user_id, sizeof(t->buy_user_id));
        strncpy(t->sell_order_id, incoming->order_id, sizeof(t->sell_order_id));
        strncpy(t->sell_user_id, incoming->user_id, sizeof(t->sell_user_id));
    }

    /* The matched price is typically the price of the order in the book,
       but it depends on your matching logic. Usually it's the best in the book
       or the incoming price, whichever logic you use. We'll use matched->price
       in this example. */
    t->price = matched->price;

    /* The size executed is the difference in quantity, but since the order
       book side executes partial fills, matched->quantity typically is how
       much was removed from that matched order in this trade. You might also
       store partial fill sizes. For a simpler approach, assume that the
       matched Orders returned each represent the actual fill size. */
    t->size = matched->quantity;

    /* Record the timestamp */
    t->timestamp = get_current_timestamp();

    return t;
}

/*
 * add_trade_to_book
 * -----------------
 * Appends a Trade to the book's executed_trades array, resizing if needed.
 */
static void add_trade_to_book(OrderBook book, const Trade t)
{
    if (!book || !t) {
        return;
    }

    /* Resize array if needed */
    if (book->trade_count == book->trade_capacity) {
        int new_capacity = (book->trade_capacity == 0) ? 8 : book->trade_capacity * 2;
        Trade *new_array = (Trade *)realloc(book->executed_trades, new_capacity * sizeof(Trade));
        if (!new_array) {
            /* If reallocation fails, we simply return (trade is lost).
               A robust system might handle this more gracefully. */
            return;
        }
        book->executed_trades = new_array;
        book->trade_capacity = new_capacity;
    }

    /* Store the Trade pointer in the array */
    book->executed_trades[book->trade_count] = t;
    book->trade_count++;
}

/*
 * get_current_timestamp
 * ---------------------
 * Returns a simple time-based timestamp. Adjust as needed for
 * higher precision or custom time sources.
 */
static time_t get_current_timestamp(void)
{
    return time(NULL);
}