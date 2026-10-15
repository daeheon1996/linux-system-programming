
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = miniShell

SRCS = src/main.c src/parser.c src/executor.c src/builtin.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
