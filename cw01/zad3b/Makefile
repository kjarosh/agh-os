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

static: dirs $(OUT)/main-static-0 $(OUT)/main-static-2 $(OUT)/main-static-3

$(OUT)/main-static-0: $(BIN)/main.0.o
	$(CC) $(CFLAGS) $(LIBDIR) -O0 -o $@ $< $(LIBS_STATIC)

$(OUT)/main-static-2: $(BIN)/main.2.o
	$(CC) $(CFLAGS) $(LIBDIR) -O2 -o $@ $< $(LIBS_STATIC)

$(OUT)/main-static-3: $(BIN)/main.3.o
	$(CC) $(CFLAGS) $(LIBDIR) -O3 -o $@ $< $(LIBS_STATIC)

shared: dirs $(OUT)/main-shared-0 $(OUT)/main-shared-2 $(OUT)/main-shared-3

$(OUT)/main-shared-0: $(BIN)/main.shared.0.o
	$(CC) $(CFLAGS) $(LIBDIR) -O0 -o $@ $< $(LIBS_SHARED) -Wl,-rpath='$$ORIGIN'

$(OUT)/main-shared-2: $(BIN)/main.shared.2.o
	$(CC) $(CFLAGS) $(LIBDIR) -O2 -o $@ $< $(LIBS_SHARED) -Wl,-rpath='$$ORIGIN'

$(OUT)/main-shared-3: $(BIN)/main.shared.3.o
	$(CC) $(CFLAGS) $(LIBDIR) -O3 -o $@ $< $(LIBS_SHARED) -Wl,-rpath='$$ORIGIN'

dynamic: dirs $(OUT)/main-dynamic-0 $(OUT)/main-dynamic-2 $(OUT)/main-dynamic-3

$(OUT)/main-dynamic-0: $(BIN)/main.dyn.0.o | $(OUT)/libchalloc.so
	$(CC) $(CFLAGS) -O0 -o $@ $< -ldl

$(OUT)/main-dynamic-2: $(BIN)/main.dyn.2.o | $(OUT)/libchalloc.so
	$(CC) $(CFLAGS) -O2 -o $@ $< -ldl

$(OUT)/main-dynamic-3: $(BIN)/main.dyn.3.o | $(OUT)/libchalloc.so
	$(CC) $(CFLAGS) -O3 -o $@ $< -ldl

$(BIN)/%.dyn.0.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O0 -DDYNAMIC_LIB -c -o $@ $<

$(BIN)/%.dyn.2.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O2 -DDYNAMIC_LIB -c -o $@ $<

$(BIN)/%.dyn.3.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O3 -DDYNAMIC_LIB -c -o $@ $<

$(BIN)/%.shared.0.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O0 -DSHARED_LIB -c -o $@ $<

$(BIN)/%.shared.2.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O2 -DSHARED_LIB -c -o $@ $<

$(BIN)/%.shared.3.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O3 -DSHARED_LIB -c -o $@ $<

$(BIN)/%.0.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O0 -c -o $@ $<

$(BIN)/%.2.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O2 -c -o $@ $<

$(BIN)/%.3.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -O3 -c -o $@ $<

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

TEST_OUT=./results3b.txt

TEST_BLK_COUNT=4096
TEST_TIMES=4096

TESTS_STATIC=\
	add $(TEST_TIMES) \
	search_element 1 1 \
	remove $(TEST_TIMES) \
	remove_and_add $(TEST_TIMES)

TEST_BLK_SIZE=4096
TESTS_DYNAMIC=\
	create_table $(TEST_BLK_COUNT) $(TEST_BLK_SIZE) $(TEST_TIMES) \
	$(TESTS_STATIC)

test:
	echo -n "" > $(TEST_OUT)
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O0\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-0 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-0 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-0 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O2\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-2 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-2 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-2 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "\n== DYNAMIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O3\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-3 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-3 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-3 dynamic $(TESTS_DYNAMIC) | tee -a ../$(TEST_OUT)
	
	echo "\n== STATIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O0\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-0 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-0 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-0 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "\n== STATIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O2\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-2 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-2 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-2 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "\n== STATIC ALLOCATION ==" >> $(TEST_OUT)
	echo "opt: -O3\n" >> $(TEST_OUT)
	
	echo "main-static:" >> $(TEST_OUT)
	cd out && \
	./main-static-3 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-shared:" >> $(TEST_OUT)
	cd out && \
	./main-shared-3 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)
	
	echo "main-dynamic:" >> $(TEST_OUT)
	cd out && \
	./main-dynamic-3 static $(TESTS_STATIC) | tee -a ../$(TEST_OUT)

