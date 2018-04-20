CC=gcc
CFLAGS=-std=gnu99 -Wall -ggdb -O0

SRC=src
BIN=bin
OUT=out

TARGET=ksh
TEST_TARGET1=tester1
TEST_TARGET2=tester2
OBJS=main.o interpreter.o command_runner.o
TEST_SRCS1=tester1.c
TEST_SRCS2=tester2.c

OBJS_PREF=$(addprefix $(BIN)/,$(OBJS))
TEST_SRCS_PREF1=$(addprefix $(SRC)/,$(TEST_SRCS1))
TEST_SRCS_PREF2=$(addprefix $(SRC)/,$(TEST_SRCS2))

.PHONY: clean all dirs test

all: $(OUT)/$(TARGET) $(OUT)/$(TEST_TARGET1) $(OUT)/$(TEST_TARGET2)

$(OUT)/$(TEST_TARGET1): $(TEST_SRCS_PREF1)
	$(CC) $(CFLAGS) -o $@ $^

$(OUT)/$(TEST_TARGET2): $(TEST_SRCS_PREF2)
	$(CC) $(CFLAGS) -o $@ $^

$(OUT)/$(TARGET): $(OBJS_PREF) | dirs
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/%.o: $(SRC)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN)/* $(OUT)/* 2>/dev/null || true

dirs:
	mkdir -p $(BIN) $(OUT)

test:
	$(OUT)/$(TARGET) test.ksh 10 10 || true

