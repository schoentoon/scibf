CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
CC     := gcc
BINARY := scib
DEPS   := build/main.o build/debug.o build/config.o build/irc_callbacks.o build/channel.o build/user.o

.PHONY: all clean

all: build $(DEPS) link

build:
	-mkdir -p build/push bin

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INC) -c -o build/main.o src/main.c

build/debug.o: src/debug.c include/debug.h
	$(CC) $(CFLAGS) $(INC) -c -o build/debug.o src/debug.c

build/config.o: src/config.c include/config.h
	$(CC) $(CFLAGS) $(INC) -c -o build/config.o src/config.c

build/irc_callbacks.o: src/irc_callbacks.c include/irc_callbacks.h
	$(CC) $(CFLAGS) $(INC) -c -o build/irc_callbacks.o src/irc_callbacks.c

build/channel.o: src/channel.c include/channel.h
	$(CC) $(CFLAGS) $(INC) -c -o build/channel.o src/channel.c

build/user.o: src/user.c include/user.h
	$(CC) $(CFLAGS) $(INC) -c -o build/user.o src/user.c

link: $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

clang:
	$(MAKE) CC=clang
