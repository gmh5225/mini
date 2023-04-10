TARGET     = mini

SRC        = main.c lex.c parse.c util.c types.c symbols.c # $(wildcard *.c)
OBJ        = $(SRC:.c=.o)
DEP        = $(OBJ:.o=.d)
CC         = gcc
CFLAGS     = -Wall -Werror -MMD -std=c11

all: $(TARGET)

debug: clean
debug: CFLAGS += -DDEBUG -g
debug: CFLAGS := $(filter-out -Werror, $(CFLAGS))
debug: all

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEP)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: cleandep
cleandep:
	rm -f $(DEP)

.PHONY: cleanall
cleanall: clean cleandep
