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

# Phase 3 (Java) targets
.PHONY: phase3
phase3:
	cd shiki/phase3_virtualization && ./build.sh

.PHONY: test_phase3
test_phase3:
	cd shiki/phase3_virtualization && ./run_tests.sh

.PHONY: run-phase3
run-phase3: phase3

.PHONY: run-phase3-integrate
run-phase3-integrate:
	cd shiki/phase3_virtualization && ./build.sh --integrate

# Phase 4 (Rust) targets
.PHONY: phase4
phase4:
	cd shiki/phase4_blackbox_fitting && ./build.sh

.PHONY: test_phase4
test_phase4:
	cd shiki/phase4_blackbox_fitting && ./run_tests.sh

.PHONY: run-phase4
run-phase4: phase4
	cd shiki/phase4_blackbox_fitting && ./build.sh --run

.PHONY: run-phase4-integrate
run-phase4-integrate:
	cd shiki/phase4_blackbox_fitting && ./build.sh --integrate

# Phase 5 (Rust) targets - Global Emergence
.PHONY: phase5
phase5:
	cd shiki/phase5_global_emergence && ./build.sh

.PHONY: test_phase5
test_phase5:
	cd shiki/phase5_global_emergence && ./run_tests.sh

.PHONY: run-phase5
run-phase5: phase5
	cd shiki/phase5_global_emergence && ./build.sh --run

.PHONY: run-phase5-integrate
run-phase5-integrate:
	cd shiki/phase5_global_emergence && ./build.sh --integrate

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(SHIKI_PHASE1_DIR)/*.o
	rm -f $(SHIKI_PHASE2_DIR)/*.o
	rm -f $(TARGET)
	rm -f $(PHASE1_TARGET)
	rm -f $(PHASE2_TARGET)
	rm -rf shiki/phase3_virtualization/build
	rm -rf shiki/phase4_blackbox_fitting/target
	rm -rf shiki/phase4_blackbox_fitting/build
	rm -rf shiki/phase5_global_emergence/target
	rm -rf shiki/phase5_global_emergence/build
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
	@echo "  phase3            - Build Shiki Phase 3 (Java) executable"
	@echo "  phase4            - Build Shiki Phase 4 (Rust) executable"
	@echo "  phase5            - Build Shiki Phase 5 (Rust) executable"
	@echo "  tests             - Build test executable"
	@echo "  test              - Build and run all tests"
	@echo "  test_phase3       - Build and run Phase 3 Java tests"
	@echo "  test_phase4       - Build and run Phase 4 Rust tests"
	@echo "  test_phase5       - Build and run Phase 5 Rust tests"
	@echo "  run               - Build and run main program"
	@echo "  run-phase1        - Build and run Phase 1 simulation"
	@echo "  run-phase1-integrate - Run Phase 1 with Red Magic integration"
	@echo "  run-phase2        - Build and run Phase 2 simulation"
	@echo "  run-phase2-integrate - Run Phase 2 with Phase 1 integration"
	@echo "  run-phase3        - Build and run Phase 3 simulation"
	@echo "  run-phase3-integrate - Run Phase 3 with Phase 1&2 integration"
	@echo "  run-phase4        - Build and run Phase 4 simulation"
	@echo "  run-phase4-integrate - Run Phase 4 with Phase 1-3 integration"
	@echo "  run-phase5        - Build and run Phase 5 simulation"
	@echo "  run-phase5-integrate - Run Phase 5 with Phase 1-4 integration"
	@echo "  clean             - Remove build artifacts"
	@echo "  help              - Show this help message"

