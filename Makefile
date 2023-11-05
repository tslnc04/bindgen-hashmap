TARGET = main
CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wstrict-overflow -fno-strict-aliasing -O2 -std=c2x -march=native

all: $(TARGET)

$(TARGET): src/$(TARGET).c src/hashmap.c src/siphash/siphash.c
	$(CC) $(CFLAGS) -o $(TARGET) src/$(TARGET).c src/siphash/siphash.c src/hashmap.c -I src/siphash -I src
