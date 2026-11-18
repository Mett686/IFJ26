CC = gcc
CFLAGS = -pedantic -Wall -Wextra -Isrc
LDLIBS = 
nazev = IFJ-archiv

build:

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/scanner_tester.o: tests/scanner_tester.c
	$(CC) $(CFLAGS) -c $< -o $@	


scannertest: src/scanner.o tests/scanner_tester.o #https://share.gemini.google/axGqUMS7ohMQ
	$(CC) $(CFLAGS) src/scanner.o tests/scanner_tester.o -o scanner_test $(LDLIBS)

clean:
	-del /q .\tests\IFJ26codeExamples\*output.txt

zip:
	zip -r $(nazev).zip src/ dokumentace.pdf rozdeleni Makefile tests/

.PHONY: all test clean zip scannertest