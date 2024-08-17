CC=gcc -std=c11
CFLAGS=-Wall -Wextra -ggdb

all: serve

serve: main.c serve.h
	$(CC) $(CFLAGS) -o $@ $<
