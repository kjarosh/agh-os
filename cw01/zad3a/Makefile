CC=gcc
CFLAGS=-Wall -std=c99 -I../zad1/src
LIBDIR=-L../zad1/out 
LIBS_STATIC=-l:libchalloc.a
LIBS_SHARED=-lchalloc

CHALLOC_SO=../zad1/out/libchalloc.so

SRC=../zad2/src
BIN=./bin
OUT=./out

all: static shared dynamic

clean:
	rm $(OUT)/* $(BIN)/* 2>/dev/null || true

static: dirs $(OUT)/main-static

$(OUT)/main-static: $(BIN)/main.o
	$(CC) $(CFLAGS) $(LIBDIR) -o $@ $< $(LIBS_STATIC)

shared: dirs $(OUT)/main-shared

$(OUT)/main-shared: $(BIN)/main.shared.o
	$(CC) $(CFLAGS) $(LIBDIR) -o $@ $< $(LIBS_SHARED) -Wl,-rpath='$$ORIGIN'

dynamic: dirs $(OUT)/main-dynamic

$(OUT)/main-dynamic: $(BIN)/main.dyn.o | $(OUT)/libchalloc.so
	$(CC) $(CFLAGS) -o $@ $< -ldl

$(BIN)/%.dyn.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -DDYNAMIC_LIB -c -o $@ $<

$(BIN)/%.shared.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -DSHARED_LIB -c -o $@ $<

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/libchalloc.so:
	ln -s $(shell pwd)/$(CHALLOC_SO) $(OUT)/libchalloc.so

dirs: | $(SRC) $(BIN) $(OUT)

$(SRC):
	mkdir -p $(SRC)

$(BIN):
	mkdir -p $(BIN)

$(OUT):
	mkdir -p $(OUT)

#############################################################################
# TESTING

TEST_OUT=./results3a.txt

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
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC1) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared dynamic $(TESTS_DYNAMIC1) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic dynamic $(TESTS_DYNAMIC1) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "blk size:" $(TEST_BLK_SIZE2) "\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC2) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared dynamic $(TESTS_DYNAMIC2) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic dynamic $(TESTS_DYNAMIC2) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "blk size:" $(TEST_BLK_SIZE3) "\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static dynamic $(TESTS_DYNAMIC3) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared dynamic $(TESTS_DYNAMIC3) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic dynamic $(TESTS_DYNAMIC3) | tee -a ../$(TEST_OUT)
	
	echo "\n== STATIC ALLOCATION ==" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)

