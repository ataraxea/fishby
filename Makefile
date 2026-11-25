CC = cc
CLANG = clang
CPPCHECK = cppcheck

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

ANALYZE_FLAGS = --analyze \
                -std=c11 \
                -Wall \
                -Wextra \
                -Wpedantic \
                -Werror \
                -Xanalyzer -analyzer-output=text

LINT_FLAGS = --enable=warning,style,performance,portability \
             --std=c11 \
             --language=c \
             --inconclusive \
             --error-exitcode=1 \
             --inline-suppr

TARGET = fishby
SAN_TARGET = fishby-san

TEST_PET = tests/test_pet
TEST_STATE = tests/test_state

SRC = src/main.c \
      src/fish.c \
      src/bubbles.c \
      src/aquarium.c \
      src/food.c \
      src/pet.c \
      src/state.c \
      src/cli.c \
      src/signals.c \
      src/platform_time.c \
      src/platform_terminal.c

HEADERS = include/fish.h \
          include/bubbles.h \
          include/aquarium.h \
          include/food.h \
          include/pet.h \
          include/state.h \
          include/cli.h \
          include/signals.h \
          include/platform_time.h \
          include/platform_terminal.h

.PHONY: all sanitize analyze lint test check clean

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

analyze:
	$(CLANG) $(CPPFLAGS) $(ANALYZE_FLAGS) $(SRC)

lint:
	$(CPPCHECK) $(CPPFLAGS) $(LINT_FLAGS) $(SRC)

$(TEST_PET): tests/test_pet.c src/pet.c include/pet.h Makefile
	$(CC) $(CPPFLAGS) $(SAN_CFLAGS) \
		-UNDEBUG \
		tests/test_pet.c src/pet.c \
		-o $(TEST_PET) $(SAN_LDFLAGS) -lm

$(TEST_STATE): tests/test_state.c src/state.c src/pet.c \
               include/state.h include/pet.h Makefile
	$(CC) $(CPPFLAGS) $(SAN_CFLAGS) \
		-UNDEBUG \
		tests/test_state.c src/state.c src/pet.c \
		-o $(TEST_STATE) $(SAN_LDFLAGS) -lm

test: $(TARGET) $(SAN_TARGET) $(TEST_PET) $(TEST_STATE)
	./$(TEST_PET)
	./$(TEST_STATE)
	bash tests/test_cli.sh ./$(TARGET)
	bash tests/test_cli.sh ./$(SAN_TARGET)

check: all sanitize analyze lint test

clean:
	rm -f $(TARGET) $(SAN_TARGET) $(TEST_PET) $(TEST_STATE)
