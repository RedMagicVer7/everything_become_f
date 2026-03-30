# Everything Becomes F - Makefile
# ================================
# Top-level Makefile for the Red Magic system simulation

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS =

# Source files
SRC_DIR = src
SOURCES = $(SRC_DIR)/counter.c \
          $(SRC_DIR)/electromagnetic_lock.c \
          $(SRC_DIR)/security_camera.c \
          $(SRC_DIR)/sealed_room.c \
          $(SRC_DIR)/event_system.c \
          $(SRC_DIR)/red_magic_system.c \
          $(SRC_DIR)/runtime.c

MAIN_SRC = $(SRC_DIR)/main.c
OBJECTS = $(SOURCES:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)

# Target executable
TARGET = red_magic

# Default target
.PHONY: all
all: $(TARGET)

# Build main executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Build and run tests
.PHONY: tests test
tests:
	$(MAKE) -C tests

test: tests
	./tests/run_all_tests

# Run main program
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(TARGET)
	$(MAKE) -C tests clean

# Clean everything including test artifacts
.PHONY: distclean
distclean: clean
	rm -f *.o

# Help
.PHONY: help
help:
	@echo "Everything Becomes F - Build System"
	@echo "===================================="
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build main executable (default)"
	@echo "  tests    - Build test executable"
	@echo "  test     - Build and run all tests"
	@echo "  run      - Build and run main program"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"
