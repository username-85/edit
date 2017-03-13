CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -g -O0
TARGET = edit
LDFLAGS = -lncurses
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

default: $(TARGET)
all: default

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(LDFLAGS) $(OBJECTS)

clean:
	-rm -f *.txt
	-rm -f *.o
	-rm -f $(TARGET)
