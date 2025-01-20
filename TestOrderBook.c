/* TestOrderBook.c
 *
 * Rigorous testing of the OrderBook module. Tests basic functionality,
 * crossing logic, edge cases, removal, best levels, and a stress scenario.
 *
 * Compile with:
 *   gcc -o TestOrderBook TestOrderBook.c OrderBook.c OrderBookSide.c \
 *       [other .c files] -I. -Wall -Wextra
 *
 * Run:
 *   ./TestOrderBook
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "OrderBook.h"

/* Simple pass/fail counters */
static int testsPassed = 0;
static int testsFailed = 0;

/* A quick macro for making assertions. 
 * If 'cond' is false, increment 'testsFailed' and print a message.
 */
#define ASSERT(cond, msg) \
    do { \
        if (cond) { \
            testsPassed++; \
        } else { \
            testsFailed++; \
            fprintf(stderr, "[FAIL] %s (line %d): %s\n", __func__, __LINE__, msg); \
        } \
    } while(0)

/* Helper function to create an Order on the heap with the given parameters. */
static Order createOrder(const char *order_id,
                         const char *user_id,
                         int quantity,
                         char side,          /* '1' for buy, '0' for sell */
                         double price,
                         long timestamp)
{
    Order o = (Order)malloc(sizeof(*o));
    if (o) {
        /* Clear the structure, then copy in each field. */
        memset(o, 0, sizeof(*o));
        strncpy(o->order_id, order_id, sizeof(o->order_id) - 1);
        strncpy(o->user_id, user_id, sizeof(o->user_id) - 1);
        o->quantity = quantity;
        o->side = side;
        o->price = price;
        o->timestamp = timestamp;
    }
    return o;
}

/* ===========================
 * Test: Create and Destroy
 * ===========================*/
static void test_create_destroy(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "OrderBook_create returned NULL");
    OrderBook_destroy(&book);

    /* If we reach here without crashing, it’s a basic pass */
    ASSERT(1, "OrderBook_create/destroy basic test passed");
}

/* ===========================
 * Test: Add Non-Crossing Orders
 * ===========================
 * Verifies that no trades are executed when orders don’t cross. */
static void test_add_non_crossing_orders(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_add_non_crossing_orders");

    /* Best ask = 101.00, Best bid = 0 (initially) */
    Order ask1 = createOrder("ask1", "alice", 100, '0', 101.0, time(NULL));
    int trade_count = -1;
    char **trade_ids = OrderBook_add_order(book, ask1, &trade_count);
    free(ask1);

    /* Expect 0 trades executed */
    ASSERT(trade_ids == NULL, "No trades should be executed for non-crossing ask");
    ASSERT(trade_count == 0, "trade_count should be 0 for non-crossing ask");

    /* Best bid = 99.00, doesn't cross the ask at 101.0 */
    Order bid1 = createOrder("bid1", "bob", 50, '1', 99.0, time(NULL));
    trade_count = -1;
    trade_ids = OrderBook_add_order(book, bid1, &trade_count);
    free(bid1);

    /* Expect 0 trades executed */
    ASSERT(trade_ids == NULL, "No trades should be executed for non-crossing bid");
    ASSERT(trade_count == 0, "trade_count should be 0 for non-crossing bid");

    /* Check best bid / best ask. */
    double best_bid = OrderBook_get_best_bid(book);
    double best_ask = OrderBook_get_best_ask(book);
    ASSERT(best_bid == 99.0, "Best bid should be 99.0");
    ASSERT(best_ask == 101.0, "Best ask should be 101.0");

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Add Crossing Orders
 * ===========================
 * Checks that trades are executed when a new order crosses the spread. */
static void test_add_crossing_orders(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_add_crossing_orders");

    /* Add an ask @ 100.0 for quantity 100 */
    Order ask1 = createOrder("ask1", "seller1", 100, '0', 100.0, time(NULL));
    int trade_count = -1;
    char **trade_ids = OrderBook_add_order(book, ask1, &trade_count);
    free(ask1);
    ASSERT(trade_ids == NULL && trade_count == 0, "No trade expected on first ask insertion");

    /* Add a bid @ 101.0 for quantity 50; it crosses the ask at 100.0 */
    Order bid1 = createOrder("bid1", "buyer1", 50, '1', 101.0, time(NULL));
    trade_ids = OrderBook_add_order(book, bid1, &trade_count);
    free(bid1);

    /* Expect a crossing trade. The incoming buy (101.0) meets ask (100.0). */
    ASSERT(trade_ids != NULL, "Should have a trade when crossing");
    ASSERT(trade_count == 1, "Expect exactly 1 trade from crossing");
    if (trade_ids) {
        /* Free each trade ID string */
        for (int i = 0; i < trade_count; i++) {
            free(trade_ids[i]);
        }
        /* Free the array */
        free(trade_ids);
    }

    /*
     * Now the ask side had 100 quantity, 50 of which was filled by the crossing bid.
     * Remaining ask quantity = 50 (at ask1's price of 100.0).
     */
    double best_ask = OrderBook_get_best_ask(book);
    ASSERT(best_ask == 100.0, "Remaining ask price should still be 100.0");

    /* If we add another crossing bid for 100 quantity @ 101, 
     * it should trade fully against the 50 left at 100.0, 
     * then 50 quantity remains from the new bid. */
    Order bid2 = createOrder("bid2", "buyer2", 100, '1', 101.0, time(NULL));
    trade_ids = OrderBook_add_order(book, bid2, &trade_count);
    free(bid2);

    /* Expect exactly 1 trade (the 50 leftover from ask1 is filled). */
    ASSERT(trade_ids != NULL, "Should have a trade again");
    ASSERT(trade_count == 1, "Expect exactly 1 trade from second crossing bid");
    if (trade_ids) {
        for (int i = 0; i < trade_count; i++) {
            free(trade_ids[i]);
        }
        free(trade_ids);
    }

    /* Now the original ask should be fully filled and removed. 
     * The 2nd crossing bid (bid2) should have 50 quantity left at 101.0. */
    double best_bid = OrderBook_get_best_bid(book);
    best_ask = OrderBook_get_best_ask(book); /* Should now be 0 if no other asks remain. */
    ASSERT(best_bid == 101.0, "New best bid should be 101.0 after partial fill remains");
    ASSERT(best_ask == 0.0,   "No asks should remain, best ask is 0.0");

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Remove Orders
 * =========================== */
static void test_remove_orders(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_remove_orders");

    /* Add a couple of orders. */
    Order bid1 = createOrder("bid1", "bob", 100, '1', 99.0, time(NULL));
    OrderBook_add_order(book, bid1, NULL);
    free(bid1);

    Order ask1 = createOrder("ask1", "alice", 100, '0', 101.0, time(NULL));
    OrderBook_add_order(book, ask1, NULL);
    free(ask1);

    /* Remove an existing order. */
    int removed = OrderBook_remove_order(book, "bid1");
    ASSERT(removed == 1, "Should successfully remove bid1");

    /* Trying to remove the same order again should fail. */
    removed = OrderBook_remove_order(book, "bid1");
    ASSERT(removed == 0, "Removing non-existing order should fail (0)");

    /* Remove ask1. */
    removed = OrderBook_remove_order(book, "ask1");
    ASSERT(removed == 1, "Should successfully remove ask1");

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Best Bid / Best Ask
 * =========================== */
static void test_best_bid_ask(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_best_bid_ask");

    /* Initially, no orders => best_bid = best_ask = 0 */
    ASSERT(OrderBook_get_best_bid(book) == 0.0, "No best bid initially");
    ASSERT(OrderBook_get_best_ask(book) == 0.0, "No best ask initially");

    /* Add a few orders at different prices. */
    Order ask1 = createOrder("ask1", "alice", 10, '0', 100.0, time(NULL));
    OrderBook_add_order(book, ask1, NULL);
    free(ask1);

    Order ask2 = createOrder("ask2", "alice", 20, '0', 105.0, time(NULL));
    OrderBook_add_order(book, ask2, NULL);
    free(ask2);

    Order bid1 = createOrder("bid1", "bob",   50, '1', 90.0,  time(NULL));
    OrderBook_add_order(book, bid1, NULL);
    free(bid1);

    Order bid2 = createOrder("bid2", "bob",   25, '1', 95.0,  time(NULL));
    OrderBook_add_order(book, bid2, NULL);
    free(bid2);

    double best_bid = OrderBook_get_best_bid(book);
    double best_ask = OrderBook_get_best_ask(book);
    ASSERT(best_bid == 95.0,  "Best bid should be 95.0");
    ASSERT(best_ask == 100.0, "Best ask should be 100.0");

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Top Levels
 * ===========================
 * Tests the OrderBook_get_top_levels function. */
static void test_top_levels(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_top_levels");

    /* Add multiple bids. */
    double bid_prices[] = { 95.0, 96.0, 97.0, 98.0 };
    for (int i = 0; i < 4; i++) {
        char bid_id[16];
        snprintf(bid_id, sizeof(bid_id), "bid%d", i);
        Order b = createOrder(bid_id, "bob", 10 * (i+1), '1', bid_prices[i], time(NULL));
        OrderBook_add_order(book, b, NULL);
        free(b);
    }

    /* Add multiple asks. */
    double ask_prices[] = { 100.0, 102.0, 101.0, 103.0 };
    for (int i = 0; i < 4; i++) {
        char ask_id[16];
        snprintf(ask_id, sizeof(ask_id), "ask%d", i);
        Order a = createOrder(ask_id, "alice", 5 * (i+1), '0', ask_prices[i], time(NULL));
        OrderBook_add_order(book, a, NULL);
        free(a);
    }

    /* Retrieve the top 2 levels from each side. */
    struct OrderBookLevelView *bid_levels = NULL;
    struct OrderBookLevelView *ask_levels = NULL;
    int bid_count = 0, ask_count = 0;

    int ok = OrderBook_get_top_levels(book, 2, &bid_levels, &bid_count, &ask_levels, &ask_count);
    ASSERT(ok == 1, "get_top_levels returned success");

    /*
     * For bids, we expect the two highest prices:
     *   - 98.0
     *   - 97.0
     * For asks, we expect the two lowest prices:
     *   - 100.0
     *   - 101.0
     */
    ASSERT(bid_count == 2, "Bid count should be 2 for top 2 levels");
    if (bid_count == 2) {
        ASSERT(bid_levels[0].price == 98.0, "Highest bid level price = 98.0");
        ASSERT(bid_levels[1].price == 97.0, "Next bid level price = 97.0");
    }

    ASSERT(ask_count == 2, "Ask count should be 2 for top 2 levels");
    if (ask_count == 2) {
        ASSERT(ask_levels[0].price == 100.0, "Lowest ask level price = 100.0");
        ASSERT(ask_levels[1].price == 101.0, "Next ask level price = 101.0");
    }

    /* Clean up dynamic allocations. */
    free(bid_levels);
    free(ask_levels);

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Get All Trades
 * ===========================
 * Verifies that we can retrieve a chronological list of all executed trades. */
static void test_get_all_trades(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_get_all_trades");

    /* Create a scenario with multiple trades. */
    /* Example: Add an ask, then add multiple crossing bids. */
    Order ask1 = createOrder("ask1", "alice", 30, '0', 100.0, time(NULL));
    OrderBook_add_order(book, ask1, NULL);
    free(ask1);

    /* 1st crossing bid for 10 => partial fill */
    Order bid1 = createOrder("bid1", "bob", 10, '1', 101.0, time(NULL));
    OrderBook_add_order(book, bid1, NULL);
    free(bid1);

    /* 2nd crossing bid for 20 => completely fill remainder of ask (20 left) */
    Order bid2 = createOrder("bid2", "bob", 20, '1', 101.0, time(NULL));
    OrderBook_add_order(book, bid2, NULL);
    free(bid2);

    /* Now the ask1 is fully filled in 2 trades. */

    /* 3rd crossing bid for 50 => no ask left, so it posts to the book. */
    Order bid3 = createOrder("bid3", "charlie", 50, '1', 101.0, time(NULL));
    OrderBook_add_order(book, bid3, NULL);
    free(bid3);

    /* Retrieve all trades. */
    Trade *all_trades = NULL;
    int tcount = 0;
    int ok = OrderBook_get_all_trades(book, &all_trades, &tcount);

    ASSERT(ok == 1, "get_all_trades returned success");
    ASSERT(tcount == 2, "Exactly 2 trades should have executed");

    if (tcount == 2) {
        /* The trades must be in chronological order:
         * 1) Fill of 10
         * 2) Fill of 20
         */
        ASSERT(all_trades[0]->size == 10, "First trade size = 10");
        ASSERT(all_trades[1]->size == 20, "Second trade size = 20");
        /* Possibly check the trade_id, buy_order_id, etc. */
    }

    /* Clean up returned trades */
    for (int i = 0; i < tcount; i++) {
        free(all_trades[i]);
    }
    free(all_trades);

    OrderBook_destroy(&book);
}

/* ===========================
 * Test: Stress / Random
 * ===========================
 * Adds a large number of random orders to the book.
 * This is a simplistic stress test. */
static void test_stress(void)
{
    /* You can adjust these parameters to scale your test. */
    const int NUM_ORDERS = 5000;
    srand((unsigned int)time(NULL));

    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "OrderBook_create failed in stress test");

    for (int i = 0; i < NUM_ORDERS; i++) {
        char side = (rand() % 2) ? '1' : '0';  /* 50/50 buy or sell */
        double price = 50.0 + (rand() % 101);  /* random price [50..150] */
        int quantity = 1 + (rand() % 100);     /* random quantity [1..100] */

        char order_id[32];
        snprintf(order_id, sizeof(order_id), "ord%d", i);

        char user_id[32];
        snprintf(user_id, sizeof(user_id), "user%d", rand() % 100);

        Order o = createOrder(order_id, user_id, quantity, side, price, time(NULL));
        if (!o) {
            /* If we can’t allocate, skip. */
            continue;
        }

        int dummy_count = 0;
        char **trade_ids = OrderBook_add_order(book, o, &dummy_count);

        /* Clean up trades if any. */
        if (trade_ids) {
            for (int j = 0; j < dummy_count; j++) {
                free(trade_ids[j]);
            }
            free(trade_ids);
        }

        free(o);
    }

    /* Retrieve all trades executed so far. */
    Trade *all_trades = NULL;
    int tcount = 0;
    int ok = OrderBook_get_all_trades(book, &all_trades, &tcount);
    ASSERT(ok == 1, "OrderBook_get_all_trades succeeded in stress test");
    /* Not verifying correctness of random trades, just ensuring we didn’t crash. */

    /* Clean up trades. */
    for (int i = 0; i < tcount; i++) {
        free(all_trades[i]);
    }
    free(all_trades);

    OrderBook_destroy(&book);

    /* If we finished without crashing, we consider the stress test “passed”. */
    ASSERT(1, "Stress test completed without errors");
}


/**
 * Test: OrderBook_get_trade
 * -------------------------
 * Ensures we can retrieve a trade by its trade_id after it is executed.
 */
static void test_get_trade(void)
{
    OrderBook book = OrderBook_create();
    ASSERT(book != NULL, "Failed to create OrderBook in test_get_trade");

    /* 1) Add an ask order. */
    Order ask = createOrder("ask1", "alice", 10, '0', 100.0, time(NULL));
    OrderBook_add_order(book, ask, NULL);
    free(ask);

    /* 2) Add a crossing bid that fully matches the ask. */
    Order bid = createOrder("bid1", "bob", 10, '1', 101.0, time(NULL));
    int trade_count = 0;
    char **trade_ids = OrderBook_add_order(book, bid, &trade_count);
    free(bid);

    /* Verify that exactly one trade was executed. */
    ASSERT(trade_ids != NULL, "One trade should occur");
    ASSERT(trade_count == 1,  "Expected exactly 1 trade");

    /* 3) Retrieve the trade by its ID. */
    if (trade_ids && trade_count == 1) {
        Trade t = OrderBook_get_trade(book, trade_ids[0]);
        ASSERT(t != NULL, "Trade should be found by trade_id");

        if (t) {
            /* Check some fields for correctness. */
            ASSERT(t->size == 10, "Trade size should be 10");
            ASSERT(t->price == 100.0, "Trade price should match ask price or match logic");
            ASSERT(strcmp(t->buy_order_id, "bid1") == 0, "Trade buy_order_id should be 'bid1'");
            ASSERT(strcmp(t->sell_order_id, "ask1") == 0, "Trade sell_order_id should be 'ask1'");
            free(t); /* Always free after retrieving. */
        }

        /* Cleanup returned trade_ids array. */
        for (int i = 0; i < trade_count; i++) {
            free(trade_ids[i]);
        }
        free(trade_ids);
    }

    OrderBook_destroy(&book);
}

/* ===========================
 * MAIN: Run All Tests
 * =========================== */
int main(void)
{
    printf("=== Running OrderBook Tests ===\n");

    test_create_destroy();
    test_add_non_crossing_orders();
    test_add_crossing_orders();
    test_remove_orders();
    test_best_bid_ask();
    test_top_levels();
    test_get_all_trades();
    test_stress();
    test_get_trade();

    printf("\n--- Test Results ---\n");
    printf("Tests Passed: %d\n", testsPassed);
    printf("Tests Failed: %d\n", testsFailed);

    /* Return 0 if everything passed, 1 otherwise. */
    return (testsFailed == 0) ? 0 : 1;
}