CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_XOPEN_SOURCE=512 -D_DEFAULT_SOURCE
LFLAGS=-pthread
LIBS=-lm

TARGET=main

SRC=src
BIN=bin
OUT=out

OBJS=main.o pgm.o filter.o
OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))

.PHONY: all dirs clean

all: $(OUT)/$(TARGET)

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT)/$(TARGET): $(OBJS_PREF)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS)

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)

