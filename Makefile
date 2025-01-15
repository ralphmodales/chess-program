CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
INCLUDE_DIR = include
BIN = chess

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/board.c $(SRC_DIR)/moves.c $(SRC_DIR)/ai.c
OBJS = $(SRCS:.c=.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: all clean
