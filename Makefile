CC = cc

CPPFLAGS = -Iinclude
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS =
LDLIBS = -lncurses -lm

HARDEN_CPPFLAGS = -D_FORTIFY_SOURCE=3

HARDEN_CFLAGS = -fstack-protector-strong \
                -fstack-clash-protection \
                -fPIE \
                -Wformat=2 \
                -Werror=format-security

HARDEN_LDFLAGS = -pie \
                 -Wl,-z,relro \
                 -Wl,-z,now \
                 -Wl,-z,noexecstack

TARGET = fishby

SRC = src/main.c \
      src/fish.c \
      src/bubbles.c \
      src/aquarium.c \
      src/food.c \
      src/pet.c \
      src/state.c \
      src/cli.c \
      src/signals.c

HEADERS = include/fish.h \
          include/bubbles.h \
          include/aquarium.h \
          include/food.h \
          include/pet.h \
          include/state.h \
          include/cli.h \
          include/signals.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS) Makefile
	$(CC) $(CPPFLAGS) $(HARDEN_CPPFLAGS) \
		$(CFLAGS) $(HARDEN_CFLAGS) \
		$(SRC) -o $(TARGET) \
		$(LDFLAGS) $(HARDEN_LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET)
