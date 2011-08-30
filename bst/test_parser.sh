#!/bin/sh

. ./test-lib.sh

test_expect_success "parsing with no negative signs" '
	./postorder --test <t01_nominus >actual &&
	test_cmp actual t01_expect
'

test_expect_success "parsing with negative signs" '
	./postorder --test <t02_minus >actual &&
	test_cmp actual t02_expect
'

test_expect_success "computations" '
	./postorder --quiet <t03_compute >actual &&
	test_cmp actual t03_expect
'

test_cleanup
