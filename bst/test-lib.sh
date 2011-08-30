#!/bin/sh

TEST_CNT=0
TEST_PASS=0
TEST_FAIL=0

test_run() {
	TEST_CNT=$(($TEST_CNT + 1))
	eval "$1"
}

test_ok () {
	TEST_PASS=$(($TEST_PASS + 1))
	printf "%2d - $msg [passed]\n" $TEST_CNT
}

test_not_ok() {
	TEST_FAIL=$((TEST_FAIL + 1))
	printf "%2d - $msg [failed]\n" $TEST_CNT
}

test_cleanup() {
	rm -f actual
}

test_done() {
	test_cleanup
}

test_cmp() {
	diff -u "$1" "$2"
}

test_expect_success() {
	msg="$1"
	code="$2"
	test_run "$code"
	if [[ "$?" -eq 0 ]]
	then
		test_ok "$msg"
	else
		test_not_ok "$msg"
	fi
}
