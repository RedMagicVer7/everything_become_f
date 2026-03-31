# Tests Makefile
# ==============

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I../include -I../shiki/include
LDFLAGS =

# Source files from main project
SRC_DIR = ../src
SHIKI_SRC_DIR = ../shiki/src
LIB_SOURCES = $(SRC_DIR)/counter.c \
              $(SRC_DIR)/electromagnetic_lock.c \
              $(SRC_DIR)/security_camera.c \
              $(SRC_DIR)/sealed_room.c \
              $(SRC_DIR)/event_system.c \
              $(SRC_DIR)/red_magic_system.c \
              $(SRC_DIR)/runtime.c \
              $(SHIKI_SRC_DIR)/phase1_sandbox_escape.c \
              $(SHIKI_SRC_DIR)/phase2_ha_cluster.c

LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

# Test sources
TEST_SOURCES = test_l1_unit.c test_l2_integration.c test_l3_system.c test_phase1.c test_phase2.c run_all_tests.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Target
TARGET = run_all_tests

.PHONY: all
all: $(TARGET)

$(TARGET): $(TEST_OBJECTS) $(LIB_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET)
