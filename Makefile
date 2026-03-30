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

# Shiki Phase 2 sources
SHIKI_PHASE2_DIR = shiki/src
SHIKI_PHASE2_SRC = $(SHIKI_PHASE2_DIR)/phase2_ha_cluster.c
SHIKI_PHASE2_MAIN = $(SHIKI_PHASE2_DIR)/phase2_main.c
SHIKI_PHASE2_OBJ = $(SHIKI_PHASE2_SRC:.c=.o)
SHIKI_PHASE2_MAIN_OBJ = $(SHIKI_PHASE2_MAIN:.c=.o)

# Target executables
TARGET = red_magic
PHASE1_TARGET = phase1_sandbox_escape
PHASE2_TARGET = phase2_ha_cluster

# Default target
.PHONY: all
all: $(TARGET) $(PHASE1_TARGET) $(PHASE2_TARGET)

# Build main executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build Phase 1 executable
.PHONY: phase1
phase1: $(PHASE1_TARGET)

$(PHASE1_TARGET): $(OBJECTS) $(SHIKI_PHASE1_OBJ) $(SHIKI_PHASE1_MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build Phase 2 executable
.PHONY: phase2
phase2: $(PHASE2_TARGET)

$(PHASE2_TARGET): $(OBJECTS) $(SHIKI_PHASE1_OBJ) $(SHIKI_PHASE2_OBJ) $(SHIKI_PHASE2_MAIN_OBJ)
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

# Run Phase 2 program
.PHONY: run-phase2
run-phase2: phase2
	./$(PHASE2_TARGET)

# Run Phase 2 with Phase 1 integration
.PHONY: run-phase2-integrate
run-phase2-integrate: phase2
	./$(PHASE2_TARGET) --integrate

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(SHIKI_PHASE1_DIR)/*.o
	rm -f $(SHIKI_PHASE2_DIR)/*.o
	rm -f $(TARGET)
	rm -f $(PHASE1_TARGET)
	rm -f $(PHASE2_TARGET)
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
	@echo "  all               - Build all executables (default)"
	@echo "  phase1            - Build Shiki Phase 1 executable"
	@echo "  phase2            - Build Shiki Phase 2 executable"
	@echo "  tests             - Build test executable"
	@echo "  test              - Build and run all tests"
	@echo "  run               - Build and run main program"
	@echo "  run-phase1        - Build and run Phase 1 simulation"
	@echo "  run-phase1-integrate - Run Phase 1 with Red Magic integration"
	@echo "  run-phase2        - Build and run Phase 2 simulation"
	@echo "  run-phase2-integrate - Run Phase 2 with Phase 1 integration"
	@echo "  clean             - Remove build artifacts"
	@echo "  help              - Show this help message"

