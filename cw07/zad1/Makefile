CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_XOPEN_SOURCE=512 -D_DEFAULT_SOURCE
LIBS=-pthread -lrt

TARGET1=barber
TARGET2=client

SRC=src
BIN=bin
OUT=out

OBJS1=barber.o barber-shop.o
OBJS1_PREF=$(addprefix $(BIN)/,$(OBJS1))
OBJS2=client.o barber-shop.o
OBJS2_PREF=$(addprefix $(BIN)/,$(OBJS2))

.PHONY: all dirs clean

all: $(OUT)/$(TARGET1) $(OUT)/$(TARGET2)

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT)/$(TARGET1): $(OBJS1_PREF)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OUT)/$(TARGET2): $(OBJS2_PREF)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)

