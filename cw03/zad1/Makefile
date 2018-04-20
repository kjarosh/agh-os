CC=gcc
CFLAGS=-std=gnu99 -Wall -ggdb -Og -D_XOPEN_SOURCE=512

SRC=src
BIN=bin
OUT=out

TARGET=main
OBJS=main.o list_files.o

OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))

.PHONY: clean all test dirs

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
	$(OUT)/$(TARGET) test-tree \> 2016-01-01
	$(OUT)/$(TARGET) test-tree \= 2018-03-17
	$(OUT)/$(TARGET) test-tree \< 2018-03-18
	$(OUT)/$(TARGET_NFTW) test-tree \> 2016-01-01

