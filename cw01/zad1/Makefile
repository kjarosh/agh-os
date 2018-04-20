CC=gcc
AR=ar
CFLAGS=-Wall -std=c99

SRC=./src
BIN=./bin
OUT=./out

all: static shared

clean:
	rm $(OUT)/* $(BIN)/*

static: dirs $(OUT)/libchalloc.a

$(OUT)/libchalloc.a: $(BIN)/challoc.o
	$(AR) rcs $(OUT)/libchalloc.a $(BIN)/challoc.o

shared: dirs $(OUT)/libchalloc.so

$(OUT)/libchalloc.so: $(BIN)/challoc.o.pic
	$(CC) -shared -lc -o $(OUT)/libchalloc.so $(BIN)/challoc.o.pic

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN)/%.o.pic: $(SRC)/%.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

dirs: | $(SRC) $(BIN) $(OUT)

$(SRC):
	mkdir -p $(SRC)

$(BIN):
	mkdir -p $(BIN)

$(OUT):
	mkdir -p $(OUT)

