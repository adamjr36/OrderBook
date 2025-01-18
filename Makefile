# Makefile for OrderedMap test

CC = gcc
CFLAGS = -Wall -Wextra -g

# Source files
SRC = TestOrderedMap.c OrderedMap.c
OBJ = $(SRC:.c=.o)

# Output executable
# TARGET = TestOrderedMap
TARGET_MAP = TestOrderedMap
TARGET_LEVEL = TestOrderBookLevel
TARGET_HASH = TestHashTable
TARGET_SIDE = TestOrderBookSide
TARGETS = $(TARGET_MAP) $(TARGET_LEVEL) $(TARGET_HASH) $(TARGET_SIDE)

# Default rule
# all: $(TARGET)
test_map: $(TARGET_MAP)
test_obl: $(TARGET_LEVEL)
test_hash: $(TARGET_HASH)
test_side: $(TARGET_SIDE)

# $(TARGET): $(OBJ)
# 	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_LEVEL): TestOrderBookLevel.o OrderBookLevel.o
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_MAP): TestOrderedMap.o OrderedMap.o
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_HASH): TestHashTable.o HashTable.o
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_SIDE): TestOrderBookSide.o OrderBookSide.o OrderBookLevel.o OrderedMap.o HashTable.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean
