CC = gcc
CFLAGS = -I./src/include -O3 -Wall -Wextra
TARGET = memesim

SRCDIR = src
BUILDDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJECTS): $(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) -c $< -o $@

$(TARGET): $(TARGET).c $(OBJECTS)
	$(CC) $(TARGET).c $(OBJECTS) -o $@ $(CFLAGS)

.PHONY:
	clean all

clean:
	rm -rf $(TARGET) $(BUILDDIR)
