CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

all: traceroute

echo.o: echo.h echo.c

traceroute.o: traceroute.c
traceroute: traceroute.o echo.o

distclean:
	rm -f *.o traceroute
clean:
	rm -f *.o
