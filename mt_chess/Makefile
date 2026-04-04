
# Marcel Timm, RhinoDevel, 2026apr26

CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -g
LDFLAGS :=

# Automatic dependency tracking (e.g. to let header changes trigger rebuilds):
CFLAGS += -MMD -MP
-include $(OBJ:.o=.d)

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
TARGET := mt_chess

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ) $(TARGET) 

