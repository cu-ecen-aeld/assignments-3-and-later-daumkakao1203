CC ?= gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?=

all: aesdsocket

default: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o $(LDFLAGS)

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean:
	rm -f aesdsocket aesdsocket.o
