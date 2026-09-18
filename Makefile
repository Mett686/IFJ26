CC = gcc
CFLAGS = -pedantic -Wall -Wextra
LDLIBS = 
nazev = unnamed

all:

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


run: primes primes-i
	ulimit -s unlimited && ./primes
	ulimit -s unlimited && ./primes-i

clean:
	rm -f *.o *.d

zip:
	zip $(xlogin).zip *.c *.h Makefile

.PHONY: all run clean zip
