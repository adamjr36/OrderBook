# README

85% of code written with gpt-4o.

I designed the architecture/data structures/module interactions, but had GPT write most of it, including the Test*.c files. The 15% of code that I wrote comes from a little bit of debugging, as well as refactoring and adding some functionality as I walked through the code and didn't love all of it.
Most modules don't enforce some obvious invariants, such as: OrderBookSide set as the "buy side" won't check that orders added to it are actually bids.

All tests run with leaks --atExit -- [executable] to ensure no memory leaks.
