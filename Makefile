CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
CORE_SOURCES = $(wildcard $(SRCDIR)/core/*.cpp)
SOURCES = $(CORE_SOURCES) $(SRCDIR)/main.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
CORE_OBJECTS = $(CORE_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
HEADERS = $(wildcard include/*.hpp)

# Create directories
$(shell mkdir -p $(BINDIR) $(OBJDIR)/core)

# Main target
all: $(BINDIR)/scheduler

$(BINDIR)/scheduler: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(BINDIR)/scheduler_tests: $(CORE_OBJECTS) tests/scheduler_tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) tests/scheduler_tests.cpp $(CORE_OBJECTS) -o $@

test: $(BINDIR)/scheduler_tests
	./$(BINDIR)/scheduler_tests

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*

.PHONY: all clean test
