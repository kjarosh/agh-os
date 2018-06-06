CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -D_POSIX_C_SOURCE=200810L -DCONF_DATAGRAM
LFLAGS=-pthread
LIBS=-lm

TARGET1=client
TARGET2=server

SRC=src
BIN=bin
OUT=out

OBJS=connection.o request.o
OBJS1=client.o $(OBJS)
OBJS2=server.o server_accept.o server_listen.o server_pinger.o server_cllist.o $(OBJS)
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

