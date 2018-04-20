CC=gcc
CFLAGS=-Wall -std=c99 -I../zad1/src -ggdb
LIBDIR=-L../zad1/out 
LIBS_STATIC=-l:libchalloc.a

SRC=./src
BIN=./bin
OUT=./out

all: static

clean:
	rm $(OUT)/* $(BIN)/* 2>/dev/null || true

static: dirs $(OUT)/main-static

$(OUT)/main-static: $(BIN)/main.o
	$(CC) $(CFLAGS) $(LIBDIR) -o $@ $< $(LIBS_STATIC)

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

dirs: | $(SRC) $(BIN) $(OUT)

$(SRC):
	mkdir -p $(SRC)

$(BIN):
	mkdir -p $(BIN)

$(OUT):
	mkdir -p $(OUT)

#############################################################################
# TESTING

TEST_OUT=./results2.txt

TEST_BLK_COUNT=4096
TEST_TIMES=4096

TESTS_STATIC=\
	add $(TEST_TIMES) \
	search_element 1 1 \
	remove $(TEST_TIMES) \
	remove_and_add $(TEST_TIMES)

TEST_BLK_SIZE1=2048
TESTS_DYNAMIC1=\
	create_table $(TEST_BLK_COUNT) $(TEST_BLK_SIZE1) $(TEST_TIMES) \
	$(TESTS_STATIC)

TEST_BLK_SIZE2=4096
TESTS_DYNAMIC2=\
	create_table $(TEST_BLK_COUNT) $(TEST_BLK_SIZE2) $(TEST_TIMES) \
	$(TESTS_STATIC)

TEST_BLK_SIZE3=8192
TESTS_DYNAMIC3=\
	create_table $(TEST_BLK_COUNT) $(TEST_BLK_SIZE3) $(TEST_TIMES) \
	$(TESTS_STATIC)

test:
	echo -n "" > $(TEST_OUT)
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "blk size:" $(TEST_BLK_SIZE1) "\n" >> $(TEST_OUT)
	
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC1) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "blk size:" $(TEST_BLK_SIZE2) "\n" >> $(TEST_OUT)
	
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC2) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "blk size:" $(TEST_BLK_SIZE3) "\n" >> $(TEST_OUT)
	
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC3) | tee -a ../$(TEST_OUT)
	
	echo "\n== STATIC ALLOCATION ==" >> $(TEST_OUT)
	
	cd out && \
	./main-static static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
