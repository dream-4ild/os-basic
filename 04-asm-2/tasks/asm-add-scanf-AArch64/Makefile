all: test_simple

simple: test_simple/test_simple.c add_scanf.s
	gcc-13 test_simple/test_simple.c add_scanf.s -o $@

test_simple: input_simple/input_simple.txt simple
	qemu -L /usr/aarch64-linux-gnu ./simple

.PHONY: all test_simple
