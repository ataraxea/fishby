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

SAN_CFLAGS = -std=c11 \
             -Wall \
             -Wextra \
             -Wpedantic \
             -Werror \
             -O1 \
             -g3 \
             -fno-omit-frame-pointer \
             -fno-optimize-sibling-calls \
             -fsanitize=address,undefined \
             -fno-sanitize-recover=all

SAN_LDFLAGS = -fsanitize=address,undefined

TARGET = fishby
SAN_TARGET = fishby-san

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

.PHONY: all sanitize clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS) Makefile
	$(CC) $(CPPFLAGS) $(HARDEN_CPPFLAGS) \
		$(CFLAGS) $(HARDEN_CFLAGS) \
		$(SRC) -o $(TARGET) \
		$(LDFLAGS) $(HARDEN_LDFLAGS) $(LDLIBS)

sanitize: $(SAN_TARGET)

$(SAN_TARGET): $(SRC) $(HEADERS) Makefile
	$(CC) $(CPPFLAGS) $(SAN_CFLAGS) \
		$(SRC) -o $(SAN_TARGET) \
		$(LDFLAGS) $(SAN_LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) $(SAN_TARGET)
