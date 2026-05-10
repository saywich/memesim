CC = gcc
CXX = g++

CFLAGS = -I./src/include -O3 -Wall -Wextra
CXXFLAGS = -I./src/include -O3 -Wall -Wextra -std=c++17 -pthread

TARGET = simulator

SRCDIR = src
BUILDDIR = build

C_SOURCES = $(wildcard $(SRCDIR)/*.c)
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(C_SOURCES))

CXX_SOURCES = $(wildcard $(SRCDIR)/*.cpp)
CXX_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(CXX_SOURCES))

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(C_OBJECTS): $(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CXX_OBJECTS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(TARGET).cpp $(C_OBJECTS) $(CXX_OBJECTS)
	$(CXX) $(TARGET).cpp $(C_OBJECTS) $(CXX_OBJECTS) -o $@ $(CXXFLAGS)

.PHONY: clean all

clean:
	rm -rf $(TARGET) $(BUILDDIR)
