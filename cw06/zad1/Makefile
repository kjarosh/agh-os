CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_POSIX_C_SOURCE=3
# -DPOSIXQ
LIBS=-lrt

TARGET1=client
TARGET2=server

SRC=src
BIN=bin
OUT=out

OBJS1=client.o
OBJS1_PREF=$(addprefix $(BIN)/,$(OBJS1))
OBJS2=server.o
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
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)
