TARGET = mini

SRC_DIR = src
INC_DIR	= include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:.c=.o)
DEPS = $(OBJ:.o=.d)

CC = gcc
CFLAGS = -Wall -Werror -MMD -std=c11 -I./$(INC_DIR)
LDFLAGS =

all: $(TARGET)

debug: clean
debug: CFLAGS += -DDEBUG -g
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
