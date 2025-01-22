#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>   // for timestamp if desired

#include "Order.h"
#include "OrderBook.h"

// -------------------------------------------------------------------
// Helper function to convert a textual side ("buy"/"sell") to the
// char side in the Order struct: '1' for buy, '0' for sell.
// Defaults to '0' (sell) if not recognized.
// -------------------------------------------------------------------
static char convert_side(const char *side_str)
{
    if (side_str == NULL) return '0';
    if (strcasecmp(side_str, "buy") == 0) {
        return '1';
    } else {
        return '0';
    }
}

// -------------------------------------------------------------------
// Processes a single CSV line: parses the command and executes it.
// For ADD orders, uses a stack-allocated struct Order rather than
// dynamically allocating one.
// -------------------------------------------------------------------
static void process_csv_line(OrderBook book, const char *line)
{
    // Make a local, modifiable copy of line
    char buffer[256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Tokenize the line
    char *token = strtok(buffer, ",");
    if (!token) {
        fprintf(stderr, "Ignoring empty line or parse error.\n");
        return;
    }

    // Identify command
    if (strcasecmp(token, "ADD") == 0) {
        // Expected format: ADD,order_id,user_id,side,price,quantity
        char *order_id_str   = strtok(NULL, ",");
        char *user_id_str    = strtok(NULL, ",");
        char *side_str       = strtok(NULL, ",");
        char *price_str      = strtok(NULL, ",");
        char *quantity_str   = strtok(NULL, ",");

        if (!order_id_str || !user_id_str || !side_str || 
            !price_str || !quantity_str) {
            fprintf(stderr, "Invalid ADD format. Skipping line: %s\n", line);
            return;
        }

        // Create a stack-allocated Order
        struct Order stack_order;
        memset(&stack_order, 0, sizeof(stack_order));

        strncpy(stack_order.order_id, order_id_str, sizeof(stack_order.order_id) - 1);
        strncpy(stack_order.user_id,  user_id_str,  sizeof(stack_order.user_id) - 1);

        stack_order.price    = atof(price_str);
        stack_order.quantity = atoi(quantity_str);
        stack_order.side     = convert_side(side_str);
        // Example timestamp logic (simple):
        stack_order.timestamp = (long)time(NULL);

        // Now add the order to the OrderBook
        int trade_count = 0;
        // Because stack_order is a 'struct Order', pass it as (Order)&stack_order
        char **trade_ids = OrderBook_add_order(book, (Order)&stack_order, &trade_count);
        
        if (trade_ids) {
            printf("Executed %d trades when adding order %s:\n", trade_count, stack_order.order_id);
            for (int i = 0; i < trade_count; i++) {
                printf("  Trade ID: %s\n", trade_ids[i]);
                free(trade_ids[i]); // each string was dynamically allocated by the OrderBook
            }
            free(trade_ids);
        } else {
            printf("No trades executed when adding order %s.\n", stack_order.order_id);
        }

    } else if (strcasecmp(token, "REMOVE") == 0) {
        // Expected format: REMOVE,order_id
        char *order_id = strtok(NULL, ",");
        if (!order_id) {
            fprintf(stderr, "Invalid REMOVE format. Skipping line: %s\n", line);
            return;
        }

        int removed = OrderBook_remove_order(book, order_id);
        if (removed) {
            printf("Successfully removed order %s.\n", order_id);
        } else {
            printf("Order %s not found.\n", order_id);
        }

    } else if (strcasecmp(token, "SHOW_BEST") == 0) {
        // Print both best bid and best ask in one go
        double best_bid = OrderBook_get_best_bid(book);
        double best_ask = OrderBook_get_best_ask(book);
        printf("Best Bid: %.2f, Best Ask: %.2f\n", best_bid, best_ask);

    } else if (strcasecmp(token, "BEST_BID") == 0) {
        // Print only best bid
        double best_bid = OrderBook_get_best_bid(book);
        printf("Best Bid: %.2f\n", best_bid);

    } else if (strcasecmp(token, "BEST_ASK") == 0) {
        // Print only best ask
        double best_ask = OrderBook_get_best_ask(book);
        printf("Best Ask: %.2f\n", best_ask);

    } else if (strcasecmp(token, "SHOW_TOP") == 0) {
        // Expected format: SHOW_TOP,k
        char *k_str = strtok(NULL, ",");
        if (!k_str) {
            fprintf(stderr, "Invalid SHOW_TOP format. Skipping line: %s\n", line);
            return;
        }

        int k = atoi(k_str);
        struct OrderBookLevelView *bid_levels = NULL;
        struct OrderBookLevelView *ask_levels = NULL;
        int bid_count = 0;
        int ask_count = 0;

        if (OrderBook_get_top_levels(book, k,
                                     &bid_levels, &bid_count,
                                     &ask_levels, &ask_count)) {
            printf("Top %d Bid Levels:\n", (k == 0) ? -1 : k);
            for (int i = 0; i < bid_count; i++) {
                // Hypothetically, each OrderBookLevelView might contain price & size
                printf("  Price: %.2f, Size: %d\n",
                       bid_levels[i].price, bid_levels[i].size);
            }
            printf("Top %d Ask Levels:\n", (k == 0) ? -1 : k);
            for (int i = 0; i < ask_count; i++) {
                printf("  Price: %.2f, Size: %d\n",
                       ask_levels[i].price, ask_levels[i].size);
            }
            free(bid_levels);
            free(ask_levels);
        } else {
            fprintf(stderr, "Failed to retrieve top levels.\n");
        }

    } else if (strcasecmp(token, "SHOW_ALL_TRADES") == 0) {
        // Print all trades in chronological order
        Trade *trades = NULL;
        int trade_count = 0;

        if (OrderBook_get_all_trades(book, &trades, &trade_count)) {
            printf("All %d trades so far:\n", trade_count);
            for (int i = 0; i < trade_count; i++) {
                printf("  Trade ID: %s | "
                       "Buy Order: %s (User %s) | "
                       "Sell Order: %s (User %s) | "
                       "Size: %d | Price: %.2f | Timestamp: %ld\n",
                       trades[i]->trade_id,
                       trades[i]->buy_order_id,  trades[i]->buy_user_id,
                       trades[i]->sell_order_id, trades[i]->sell_user_id,
                       trades[i]->size,
                       trades[i]->price,
                       trades[i]->timestamp);
            }
            // Free each Trade struct
            for (int i = 0; i < trade_count; i++) {
                free(trades[i]);
            }
            free(trades);
        } else {
            fprintf(stderr, "Failed to retrieve all trades.\n");
        }

    } else if (strcasecmp(token, "GET_TRADE") == 0) {
        // Expected format: GET_TRADE,trade_id
        char *trade_id_str = strtok(NULL, ",");
        if (!trade_id_str) {
            fprintf(stderr, "Invalid GET_TRADE format. Skipping line: %s\n", line);
            return;
        }

        Trade t = OrderBook_get_trade(book, trade_id_str);
        if (t) {
            printf("Trade found: ID: %s | "
                   "Buy Order: %s (User %s) | "
                   "Sell Order: %s (User %s) | "
                   "Size: %d | Price: %.2f | Timestamp: %ld\n",
                   t->trade_id,
                   t->buy_order_id,  t->buy_user_id,
                   t->sell_order_id, t->sell_user_id,
                   t->size,
                   t->price,
                   t->timestamp);
            free(t);
        } else {
            printf("No trade found with ID '%s'\n", trade_id_str);
        }

    } else {
        fprintf(stderr, "Unrecognized command: %s. Skipping line.\n", token);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <csv_file1> [csv_file2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Create the OrderBook
    OrderBook book = OrderBook_create();
    if (!book) {
        fprintf(stderr, "Failed to create OrderBook.\n");
        return EXIT_FAILURE;
    }

    // Process each CSV file passed as an argument
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "Could not open file '%s'. Skipping.\n", argv[i]);
            continue;
        }

        printf("Processing file: %s\n", argv[i]);

        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            // Strip newline
            char *newline = strchr(line, '\n');
            if (newline) *newline = '\0';

            // Ignore empty lines
            if (strlen(line) == 0) {
                continue;
            }

            // Process this CSV line
            process_csv_line(book, line);
        }

        fclose(fp);
    }

    // Destroy the OrderBook
    OrderBook_destroy(&book);

    return EXIT_SUCCESS;
}