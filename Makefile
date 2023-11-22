CC = gcc
CFLAGS = -g -Wall
EXEC = jsh

SRC:=$(wildcard src/*.c)
HEADERS:=$(wildcard src/*.h)
OBJS:=$(patsubst src/%.c,build/%.o, $(SRC))

.PHONY: build build_dir clean leak run

run : build
	./$(EXEC)

$(EXEC): $(OBJS)
	$(CC) -Wall -o $@ $^ -lreadline

build: build_dir $(EXEC)

build_dir:
	mkdir -p build

build/%.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -rf build $(EXEC)


leak :
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) 
