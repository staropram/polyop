CC=clang
CFLAGS=-g -std=c99 -I/usr/local/include/libpng16 -I/usr/local/include -Wall
LDFLAGS=-lm -lpng16 -L/usr/local/lib
default: polyop polytest

polytest: polynomial.o polytest.c

polyop: polynomial.o polyop.c

clean:
	rm *.o; rm polytest; rm newset; rm polyop; rm set_b;
