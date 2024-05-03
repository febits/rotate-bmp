CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
CINCLUDE=-I./include/
CFLAGS+=$(CINCLUDE)

SRC=$(wildcard src/*.c)
BIN=rotate

.PHONY: default clean

default: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(BIN)
