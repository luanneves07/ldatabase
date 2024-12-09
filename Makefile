TARGET = bin/ldbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g -Iinclude

all: $(TARGET)
run: clean default
	./$(TARGET) -f ./my-db.db

default: all

clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) -o $@ $?

obj/%.o: src/%.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<
