AR = ar
CC = gcc
YZC = ../../amc

.PHONY: all clean

all: main

hello.o: hello.c
	$(CC) -c -o $@ $^

libhello.a: hello.o
	$(AR) rcs $@ $^

main: main.yz libhello.a
	$(YZC) -o main main.yz \
		--ld "musl-gcc" \
		--ld-flags "-static -L." \
		-l hello

clean:
	rm -rf .cache libhello.a hello.o main
