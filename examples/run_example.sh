#!/bin/bash
#
# Simple script to run the PI computation with different iteration counts
#

echo "BOINC PI Computation - Example Runner"
echo "======================================"
echo ""

# Check if application is built
if [ ! -f "./pi_compute" ]; then
    echo "Application not built. Building now..."
    make
    if [ $? -ne 0 ]; then
        echo "Build failed! Please check error messages above."
        exit 1
    fi
    echo ""
fi

# Menu for iteration count
echo "Select iteration count:"
echo "1) Quick test (1 million - ~0.1 seconds)"
echo "2) Fast (10 million - ~1 second)"
echo "3) Accurate (100 million - ~10 seconds)"
echo "4) Very accurate (1 billion - ~100 seconds)"
echo "5) Custom"
echo ""
read -p "Enter choice [1-5]: " choice

case $choice in
    1)
        ITERATIONS=1000000
        ;;
    2)
        ITERATIONS=10000000
        ;;
    3)
        ITERATIONS=100000000
        ;;
    4)
        ITERATIONS=1000000000
        ;;
    5)
        read -p "Enter number of iterations: " ITERATIONS
        ;;
    *)
        echo "Invalid choice. Using 10 million iterations."
        ITERATIONS=10000000
        ;;
esac

# Create input file
echo "$ITERATIONS" > in
echo ""
echo "Running computation with $ITERATIONS iterations..."
echo "Please wait..."
echo ""

# Run the application
START_TIME=$(date +%s)
./pi_compute > /dev/null 2>&1
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

# Display results
echo "==============================================="
echo "Computation completed in $ELAPSED seconds"
echo "==============================================="
echo ""
cat out
echo ""

# Check if checkpoint was created
if [ -f "checkpoint.txt" ]; then
    echo "Checkpoint file created: checkpoint.txt"
fi

# Show stderr output if needed
if [ -f "stderr.txt" ]; then
    echo ""
    echo "Application messages (from stderr.txt):"
    echo "---------------------------------------"
    grep "^APP:" stderr.txt
fi

echo ""
echo "Output saved to: out"
echo "Run again with './run_example.sh'"
