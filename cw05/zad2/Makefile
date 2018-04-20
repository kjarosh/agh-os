CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_POSIX_C_SOURCE=200810L -D_DEFAULT_SOURCE -D_GNU_SOURCE

TARGET1=master
TARGET2=slave

SRC=src
BIN=bin
OUT=out

OBJS1=master.o
OBJS1_PREF=$(addprefix $(BIN)/,$(OBJS1))
OBJS2=slave.o
OBJS2_PREF=$(addprefix $(BIN)/,$(OBJS2))

.PHONY: all dirs clean

all: $(OUT)/$(TARGET1) $(OUT)/$(TARGET2)

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT)/$(TARGET1): $(OBJS1_PREF)
	$(CC) $(CFLAGS) -o $@ $^

$(OUT)/$(TARGET2): $(OBJS2_PREF)
	$(CC) $(CFLAGS) -o $@ $^

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)

