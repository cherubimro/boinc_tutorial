# Makefile for BOINC PI Computation Application
#
# This Makefile demonstrates how to build a BOINC application.
# Students can modify this for their own projects.

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -O2 -I/usr/include/boinc -I/usr/local/include/boinc
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

# Default target
all: $(TARGET)

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

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGET) $(VERSIONED_TARGET)
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

# Help target
help:
	@echo "BOINC PI Computation Application - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build the application (default)"
	@echo "  clean    - Remove build artifacts and test files"
	@echo "  test     - Build and run a local test"
	@echo "  install  - Show installation instructions"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - BOINC development libraries must be installed"
	@echo "  - On Ubuntu/Debian: sudo apt-get install boinc-dev"
	@echo "  - Or build BOINC from source and install libraries"

.PHONY: all clean install test help
