CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDLIBS = -lncurses

TARGET = fishby
SRC = src/main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
