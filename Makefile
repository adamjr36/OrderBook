# Makefile for compiling code in src/*.c (excluding src/Test*.c)
# Produces the executable OrderBookDriver

CC := gcc
CFLAGS := -Wall -Wextra -g #-O2 -Iinclude
EXECUTABLE := OrderBookDriver

SRC_DIR := src

# Grab all .c files in src/ excluding those starting with Test
SOURCES := $(filter-out $(SRC_DIR)/Test%.c, $(wildcard $(SRC_DIR)/*.c))
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
