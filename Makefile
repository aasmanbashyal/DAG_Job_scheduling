CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = $(wildcard $(SRCDIR)/core/*.cpp) $(SRCDIR)/main.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Create directories
$(shell mkdir -p $(BINDIR) $(OBJDIR)/core)

# Main target
all: $(BINDIR)/scheduler

$(BINDIR)/scheduler: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*

.PHONY: all clean