CC=gcc
CFLAGS=-std=gnu99 -Wall -ggdb -Og

SRC=src
BIN=bin
OUT=out

TARGET=ksh
OBJS=main.o interpreter.o command_runner.o

OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))

.PHONY: clean all dirs test

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
	$(OUT)/$(TARGET) test.ksh || true
	cat test.ksh | $(OUT)/$(TARGET) || true

