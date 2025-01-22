# README

All tests run with leaks --atExit -- [executable] to ensure no memory leaks.

Everything passes, including the driver with the minimal example.csv. Driver and .csv files can't really handle getting a trade by Trade ID because IDs are created dynamically, but this feature is tested in TestOrderBook.c.

## HashTable, OrderedMap, OrderBookLevel, OrderBookSide

80% of code written with gpt-4o.

I designed the architecture/data structures/module interactions, but had GPT write most of it, including the Test*.c files. The 15% of code that I wrote comes from a little bit of debugging, as well as refactoring and adding some functionality as I walked through the code and didn't love all of it.

Most modules don't enforce some obvious invariants, such as: OrderBookSide set as the "buy side" won't check that orders added to it are actually bids.

## OrderBook

90% written with gpt-1o. Immediately and obviously WAY better than 4o. I prompted it with OrderBookSide.h and it did a very good job of writing in the same style, but much nicer, simpler, and better commented code.

Still had issues I had to manually debug, but this was much easier due to the function contracts and inline comments 1o put in the .c file. 

There were a lot of memory leaks and the testing file did a great job of making them obvious. If OrderBookSide.h were better documented, a lot of them likely would not have occurred.

## Outstanding Issues

1. Not as much error handling as a real production application. Good enough for what it is with minimal prompting and asking for test files to be "rigorous."
2. Only thing really missing is serialization and deserialization for it to be really usable.
