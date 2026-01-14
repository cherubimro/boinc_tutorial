# Makefile for BOINC PI Computation Application
#
# This Makefile demonstrates how to build a BOINC application.
# Students can modify this for their own projects.

# Compiler settings
CXX = g++
CC = gcc
CXXFLAGS = -Wall -O2 -I/usr/include/boinc -I/usr/local/include/boinc
CFLAGS = -Wall -O2
LDFLAGS = -L/usr/lib -L/usr/local/lib
LIBS = -lboinc_api -lboinc -lpthread -lm

# Target executable name
TARGET = pi_compute
VERSION = 1.0
PLATFORM = $(shell uname -m)-pc-linux-gnu
VERSIONED_TARGET = $(TARGET)_$(VERSION)_$(PLATFORM)

# Source files
SOURCES = pi_compute.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Standalone Monte Carlo solver
SIMPLE_MC = simpleAxbMC
SIMPLE_MC_SRC = simpleAxbMC.c

# Default target
all: $(TARGET) $(SIMPLE_MC)

# Build the application
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	@echo "Build successful!"
	@echo "Creating versioned copy: $(VERSIONED_TARGET)"
	cp $(TARGET) $(VERSIONED_TARGET)

# Compile source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build standalone Monte Carlo solver
$(SIMPLE_MC): $(SIMPLE_MC_SRC)
	@echo "Building standalone Monte Carlo solver..."
	$(CC) $(CFLAGS) -o $(SIMPLE_MC) $(SIMPLE_MC_SRC) -lm
	@echo "Build successful!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGET) $(VERSIONED_TARGET)
	rm -f $(SIMPLE_MC)
	rm -f checkpoint.txt out in
	@echo "Clean complete!"

# Install target (for reference - modify paths as needed)
install: $(TARGET)
	@echo "To install, copy $(VERSIONED_TARGET) to your BOINC project's"
	@echo "apps/$(TARGET)/$(VERSION)/ directory on the server"

# Test target - creates a sample input file and runs the app locally
test: $(TARGET)
	@echo "Creating test input file..."
	@echo "10000000" > in
	@echo "Running $(TARGET) with 10 million iterations..."
	./$(TARGET)
	@echo "Test complete! Check 'out' file for results."
	@cat out

# Test standalone Monte Carlo solver
test-mc: $(SIMPLE_MC)
	@echo "Testing Monte Carlo solver with 5x5 system, 50000 walks..."
	./$(SIMPLE_MC) 5 50000
	@echo ""
	@echo "Try with more walks for better accuracy:"
	@echo "  ./$(SIMPLE_MC) 5 500000"
	@echo "Or with a larger system:"
	@echo "  ./$(SIMPLE_MC) 10 100000"

# Help target
help:
	@echo "BOINC PI Computation Application - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build all applications (default)"
	@echo "  clean    - Remove build artifacts and test files"
	@echo "  test     - Build and run PI computation test"
	@echo "  test-mc  - Build and run Monte Carlo solver test"
	@echo "  install  - Show installation instructions"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Standalone programs:"
	@echo "  $(SIMPLE_MC) - Ulam-von Neumann Monte Carlo solver"
	@echo "    Usage: ./$(SIMPLE_MC) [dimension] [num_walks]"
	@echo "    Example: ./$(SIMPLE_MC) 10 100000"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - BOINC development libraries must be installed"
	@echo "  - On Ubuntu/Debian: sudo apt-get install boinc-dev"
	@echo "  - Or build BOINC from source and install libraries"

.PHONY: all clean install test test-mc help
