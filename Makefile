TARGET     = mini

SRC        = mini.c
CC         = gcc
CFLAGS     = -Wall -Werror -MMD -std=c11

all: $(TARGET)

debug: clean
debug: CFLAGS += -DDEBUG -g
debug: CFLAGS := $(filter-out -Werror, $(CFLAGS))
debug: all

$(TARGET): $(SRC)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
