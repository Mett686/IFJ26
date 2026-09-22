CC = gcc
CFLAGS = -pedantic -Wall -Wextra
LDLIBS = 
nazev = IFJ-archiv

build:

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: 
	

clean:
	rm -f *.zip *.o

zip:
	zip -r $(nazev).zip src/ dokumentace.pdf rozdeleni Makefile tests/

.PHONY: all test clean zip
