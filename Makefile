# Everything Becomes F - Makefile
# ================================
# Top-level Makefile for the Red Magic system simulation

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Ishiki/include
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

# Shiki Phase 1 sources
SHIKI_PHASE1_DIR = shiki/src
SHIKI_PHASE1_SRC = $(SHIKI_PHASE1_DIR)/phase1_sandbox_escape.c
SHIKI_PHASE1_MAIN = $(SHIKI_PHASE1_DIR)/phase1_main.c
SHIKI_PHASE1_OBJ = $(SHIKI_PHASE1_SRC:.c=.o)
SHIKI_PHASE1_MAIN_OBJ = $(SHIKI_PHASE1_MAIN:.c=.o)

# Target executables
TARGET = red_magic
PHASE1_TARGET = phase1_sandbox_escape

# Default target
.PHONY: all
all: $(TARGET)

# Build main executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build Phase 1 executable
.PHONY: phase1
phase1: $(PHASE1_TARGET)

$(PHASE1_TARGET): $(OBJECTS) $(SHIKI_PHASE1_OBJ) $(SHIKI_PHASE1_MAIN_OBJ)
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

# Run Phase 1 program
.PHONY: run-phase1
run-phase1: phase1
	./$(PHASE1_TARGET)

# Run Phase 1 with Red Magic integration
.PHONY: run-phase1-integrate
run-phase1-integrate: phase1
	./$(PHASE1_TARGET) --integrate

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(SHIKI_PHASE1_DIR)/*.o
	rm -f $(TARGET)
	rm -f $(PHASE1_TARGET)
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
	@echo "  all               - Build main executable (default)"
	@echo "  phase1            - Build Shiki Phase 1 executable"
	@echo "  tests             - Build test executable"
	@echo "  test              - Build and run all tests"
	@echo "  run               - Build and run main program"
	@echo "  run-phase1        - Build and run Phase 1 simulation"
	@echo "  run-phase1-integrate - Run Phase 1 with Red Magic integration"
	@echo "  clean             - Remove build artifacts"
	@echo "  help              - Show this help message"

