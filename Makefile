# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE -O2
LDFLAGS = -pthread -lm
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Target
TARGET = shell

# Source and Object Files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Phony Targets
.PHONY: all debug clean install test

# Default Target
all: $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Object Compilation Rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Create Object Directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Install target
install: $(TARGET)
	mkdir -p $(BINDIR)
	cp $(TARGET) $(BINDIR)/

# Test target
test: $(TARGET)
	./$(TARGET) dangerous_commands_sample.txt test.log

# Cleanup
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Full cleanup including binaries
distclean: clean
	rm -rf $(BINDIR)
