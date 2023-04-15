TARGET = mini

SRC_DIR = src
INC_DIR	= include
BUILD_DIR = build

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(patsubst $(BUILD_DIR)/%.o,$(BUILD_DIR)/%.d,$(OBJS))

CC = gcc
CFLAGS = -Wall -Werror -MMD -std=c11 -I./$(INC_DIR)
LDFLAGS =

all: $(TARGET)

debug: clean
debug: CFLAGS += -DDEBUG -g
debug: CFLAGS := $(filter-out -Werror, $(CFLAGS))
debug: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

-include $(DEP)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
