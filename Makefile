CC = gcc
CFLAGS = -pedantic -Wall -Wextra -Isrc -MMD -MP
LDLIBS =
nazev = IFJ-archiv

SRCS = \
	src/main.c \
	src/scanner.c \
	src/stack.c \
	src/parser.c \
	src/codegen.c \
	src/htab.c \
	src/abtDecoding.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

all: build

build: compiler

compiler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/scanner_tester.o: tests/scanner_tester.c
	$(CC) $(CFLAGS) -c $< -o $@

scannertest: src/scanner.o tests/scanner_tester.o
	$(CC) $(CFLAGS) src/scanner.o tests/scanner_tester.o -o scanner_test $(LDLIBS)

test: scannertest
	./scanner_test

zip:
	zip -r $(nazev).zip src/ dokumentace.pdf rozdeleni Makefile tests/

clean:
	rm -f $(OBJS) $(DEPS) tests/scanner_tester.o tests/scanner_tester.d scanner_test $(nazev).zip

-include $(DEPS) tests/scanner_tester.d

.PHONY: all build test clean zip scannertest