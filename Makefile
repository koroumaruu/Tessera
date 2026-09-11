CC     = clang
CFLAGS = -g -Wall -Wextra -Iinclude $(shell pkg-config --cflags raylib)
LFLAGS = $(shell pkg-config --libs raylib) -lm -lX11

TARGET = bin/tessera
SRCS   = $(wildcard src/*.c)
OBJS   = $(patsubst src/%.c, build/%.o, $(SRCS))

RAYLIB_SRC = libs/raylib/src

$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) $(OBJS) $(LFLAGS) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

compile_commands.json:
	bear -- make

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(RAYLIB_SRC)/*.o
	rm -f $(RAYLIB_SRC)/platforms/*.o
