
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -pthread

TARGET = http_server
SRCS = src/main.c src/server.c src/http.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -pthread

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
