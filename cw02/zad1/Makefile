SHELL:=/bin/bash

CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -O0

SRC=src
BIN=bin
OUT=out

TARGET=main
OBJS=main.o operations.o file_desc.o

OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))

.PHONY: clean all test

all: $(OUT)/$(TARGET)

$(OUT)/$(TARGET): $(OBJS_PREF) | dirs
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

dirs:
	mkdir -p $(BIN) $(OUT)

test:
	bash test.sh $(OUT)/$(TARGET)
