CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic

all: traceroute

traceroute.o: traceroute.c
traceroute: traceroute.o

clean:
	rm *.o traceroute
