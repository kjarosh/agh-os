CC=gcc
CFLAGS=-std=c99 -Wall -D_POSIX_C_SOURCE=200810L -D_DEFAULT_SOURCE

TARGET=main

SRC=src
BIN=bin
OUT=out

OBJS=main.o
OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))

.PHONY: all dirs clean

all: $(OUT)/$(TARGET)

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT)/$(TARGET): $(OBJS_PREF)
	$(CC) $(CFLAGS) -o $@ $^

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)

