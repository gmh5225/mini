TARGET = mini

SRC_DIR = src
INC_DIR	= include
LIB_DIR = minigen

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:.c=.o)
DEPS = $(OBJ:.o=.d)

CC = gcc
CFLAGS = -Wall -Werror -MMD -std=c11 -I./$(INC_DIR)
LDFLAGS = -lminigen
LDFLAGS_DEBUG = -L$(LIB_DIR)/build -lminigen

all: $(TARGET)

debug: clean
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: CFLAGS += -DDEBUG -g -I$(LIB_DIR)/include
debug: CFLAGS := $(filter-out -Werror, $(CFLAGS))
debug: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: cleandep
cleandep:
	rm -f $(DEP)

.PHONY: cleanall
cleanall: clean cleandep

-include $(DEP)
