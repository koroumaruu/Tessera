CC     = clang
CFLAGS = -g -Wall -Wextra -Iinclude $(shell pkg-config --cflags raylib)
LFLAGS = $(shell pkg-config --libs raylib) -lm -lX11

TARGET = bin/tessera
SRCS   = $(wildcard src/*.c)
OBJS   = $(patsubst src/%.c, build/%.o, $(SRCS))

# Should grab root directory path dynamically across any dsitro
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

#for some reason it refuses to launch without this and my knowledge on C is absolute shit so loadbearing coconut i guess)
all: $(TARGET) launcher

$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) $(OBJS) $(LFLAGS) -o $(TARGET)
	cp -r assets bin/

#Wrapper script to execute it anywhere (Probably security vuln of doom and despair)
launcher: $(TARGET)
	@printf '#!/usr/bin/env bash\ncd "%sbin" || exit 1\nexec ./tessera "$$@"\n' "$(ROOT_DIR)" > tessera
	chmod +x tessera

#symlinks the shell script to /usr/local/bin to run globally
install: all
	@echo "Installing tessera /usr/local/bin..."
	ln -sf "$(ROOT_DIR)tessera" /usr/local/bin/tessera


build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

uninstall:
	rm -f $(OBJS) $(TARGET)
	rm -rf build bin tessera
	rm -f /usr/local/bin/tessera
