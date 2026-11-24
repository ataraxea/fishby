CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -Iinclude
LDLIBS = -lncurses -lm

TARGET = fishby

SRC = src/main.c \
      src/fish.c \
      src/bubbles.c

HEADERS = include/fish.h \
          include/bubbles.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
