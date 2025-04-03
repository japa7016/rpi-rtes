# do make for compilation without error
# Doing make strict would not compile
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = hello_world
OBJECTS = hello_world.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

strict: CFLAGS += -Werror
strict: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)
