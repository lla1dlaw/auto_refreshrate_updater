# Compiler and flags
CC = gcc
PKG_CFLAGS := $(shell pkg-config --cflags glib-2.0 upower-glib)
PKG_LDFLAGS := $(shell pkg-config --libs glib-2.0 upower-glib)
CFLAGS = -Wall -Wextra -g -Iupdater/include -Iconfigurator/src/cJSON $(PKG_CFLAGS)
LDFLAGS = -lX11 -lXrandr -lm $(PKG_LDFLAGS)

# Target executable
TARGET = auto_refreshrate_updater
ROOTDIR := ./

SRCS := $(shell find $(ROOTDIR) -type f -name '*.c')


# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean