CC = gcc
SOURCE_PATH = $(if $(wildcard src/main.c),src/,)
CFLAGS = -pedantic -Wall -Wextra $(if $(SOURCE_PATH),-I$(SOURCE_PATH),-I.) -MMD -MP
LDLIBS =
nazev = xknotem00
INTERPRETER ?= ./interpret

SOURCE_NAMES = \
	main.c \
	scanner.c \
	stack.c \
	parser.c \
	codegen.c \
	htab.c \
	abtDecoding.c

SRCS = $(addprefix $(SOURCE_PATH),$(SOURCE_NAMES))
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
TEST_OBJS = tests/scanner_tester.o tests/parser_tests.o tests/AST_tests.o
TEST_DEPS = $(TEST_OBJS:.o=.d)
PARSER_TEST_OBJS = $(addprefix $(SOURCE_PATH),parser.o stack.o htab.o abtDecoding.o codegen.o)

all: build

build: compiler

compiler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

scannertest: $(SOURCE_PATH)scanner.o tests/scanner_tester.o
	$(CC) $(CFLAGS) $(SOURCE_PATH)scanner.o tests/scanner_tester.o -o scanner_test $(LDLIBS)

parser_test: tests/parser_tests.o $(PARSER_TEST_OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

ast_test: tests/AST_tests.o $(PARSER_TEST_OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

parser-test: parser_test
	./parser_test

ast-test: ast_test
	./ast_test

scanner-test: scannertest
	./scanner_test < tests/final_tests/factorial.scala > /dev/null

test: scanner-test parser-test ast-test test-final

test-final: compiler
	INTERPRETER="$(INTERPRETER)" ./tests/run_final_tests.sh

zip:
ifeq ($(OS),Windows_NT)
	powershell -NoProfile -Command "Compress-Archive -Path 'src\*', 'Makefile' -DestinationPath $(nazev).zip -Force"
else
	zip -j -r $(nazev).zip src/ Makefile
endif

check: zip
	rm -f -r tmp_testdir
	mkdir tmp_testdir
	./is_it_ok.sh $(nazev).zip tmp_testdir
	rm -f -r tmp_testdir

doc:
	pdflatex dokumentace.tex

clean:
	rm -f $(OBJS) $(DEPS) $(TEST_OBJS) $(TEST_DEPS) scanner_test parser_test ast_test test_runner htab_test $(nazev).zip

-include $(DEPS) $(TEST_DEPS)

.PHONY: all build test test-final scanner-test parser-test ast-test clean zip scannertest documentation check
