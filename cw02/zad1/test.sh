#!/bin/bash

results=wyniki.txt
record_count_sort=(2048 4096)
record_count_copy=(32768 65536)

function run_test_sort {
	for rcount in ${record_count_sort[@]}; do
		echo "record count:" $rcount
		echo "record size:" $2
		
		$1 generate test1.gen $rcount $2;
		cp test1.gen test2.gen
		
		printf "LIB: "
		time $1 sort test1.gen $rcount $2 lib;
		
		printf "SYS: "
		time $1 sort test2.gen $rcount $2 sys;
		
		rm test1.gen test2.gen;
		
		echo ""
	done
}

function run_test_copy {
	for rcount in ${record_count_copy[@]}; do
		echo "record count:" $rcount
		echo "record size:" $2
		
		$1 generate test.gen $rcount $2;
		
		printf "LIB: "
		time $1 copy test.gen test1.gen $rcount $2 lib;
		
		printf "SYS: "
		time $1 copy test.gen test2.gen $rcount $2 sys;
		
		rm test.gen test1.gen test2.gen;
		
		echo ""
	done
}

echo -n "" > $results

TIMEFORMAT="r:%R u:%U s:%S"

echo "===========================" | tee -a $results;
echo "operation: sorting"          | tee -a $results;
echo ""                            | tee -a $results;
run_test_sort $1 4    2>&1 | tee -a $results;
run_test_sort $1 512  2>&1 | tee -a $results;
run_test_sort $1 4096 2>&1 | tee -a $results;
run_test_sort $1 8192 2>&1 | tee -a $results;

echo "===========================" | tee -a $results;
echo "operation: copying"          | tee -a $results;
echo ""                            | tee -a $results;
run_test_copy $1 4    2>&1 | tee -a $results;
run_test_copy $1 512  2>&1 | tee -a $results;
run_test_copy $1 4096 2>&1 | tee -a $results;
run_test_copy $1 8192 2>&1 | tee -a $results;
