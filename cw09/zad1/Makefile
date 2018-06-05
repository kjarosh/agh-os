CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_XOPEN_SOURCE=512 -D_POSIX_C_SOURCE=200810L -D_GNU_SOURCE
LFLAGS=-pthread
LIBS=-lm

TARGET1=main-monitor
TARGET2=main-sem

SRC=src
BIN=bin
OUT=out

OBJS=cl.o runner.o buffer.o logger.o
OBJS1=main-monitor.o $(OBJS)
OBJS2=main-sem.o $(OBJS)
OBJS1_PREF=$(addprefix $(BIN)/,$(OBJS1))
OBJS2_PREF=$(addprefix $(BIN)/,$(OBJS2))

.PHONY: all dirs clean

all: $(OUT)/$(TARGET1) $(OUT)/$(TARGET2)

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT)/$(TARGET1): $(OBJS1_PREF)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS)

$(OUT)/$(TARGET2): $(OBJS2_PREF)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS)

dirs: | $(OUT) $(BIN)

$(OUT) $(BIN):
	mkdir -p $(OUT) $(BIN)

