CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Isrc -Iinclude
SRCS    = $(wildcard src/*.c)
OUT     = pokecim

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
