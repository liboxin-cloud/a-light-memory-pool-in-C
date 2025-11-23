CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread
TARGET = mem_pool_test
SRC = test.c

all: $(TARGET)

$(TARGET): $(SRC) memory_pool.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

rebuild: clean all

.PHONY: all run clean rebuild
