CC ?= cc
CFLAGS ?= -std=gnu99 -Wall -Wextra
TARGET := ege-shell

.PHONY: all clean

all: $(TARGET)

$(TARGET): shell.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET)
