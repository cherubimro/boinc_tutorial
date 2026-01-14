#!/bin/bash
#
# test_axb_montecarlo.sh
#
# Test script for the Ax=b Monte Carlo solver
# Generates a test problem and runs the solver
#
# Licensed under GPL v3

set -e

echo "======================================"
echo "Testing Ax=b Monte Carlo Solver"
echo "======================================"
echo

# Generate test work units
echo "Generating test matrix and work units..."
python3 ../tools/generate_axb_work.py \
    --dimension 5 \
    --num-work-units 3 \
    --num-walks 50000 \
    --output-dir ./axb_test \
    --save-matrix ./axb_test/matrix.txt

echo
echo "Building solver..."
cd ../src
gcc -o axb_montecarlo Axb-MonteCarlo.c -lm
cd ../examples

echo
echo "Running work units..."
mkdir -p axb_test/results

for wu_input in axb_test/axb_wu_*_input.txt; do
    wu_name=$(basename "$wu_input" _input.txt)
    echo "  Processing $wu_name..."

    ../src/axb_montecarlo "$wu_input" "axb_test/results/${wu_name}_output.txt" \
        > "axb_test/results/${wu_name}.log" 2>&1
done

echo
echo "Results from each work unit:"
for result in axb_test/results/axb_wu_*_output.txt; do
    echo "--- $(basename $result) ---"
    cat "$result"
    echo
done

echo "Merging partial solutions..."
python3 << 'PYTHON_SCRIPT'
import numpy as np
import glob

# Read all partial results
results = []
for result_file in sorted(glob.glob('axb_test/results/axb_wu_*_output.txt')):
    with open(result_file, 'r') as f:
        lines = f.readlines()
        start_idx, end_idx = map(int, lines[0].split())
        values = [float(line.strip()) for line in lines[1:]]
        results.append((start_idx, end_idx, values))

# Merge into complete solution
n = max(end for _, end, _ in results) + 1
x_computed = np.zeros(n)

for start_idx, end_idx, values in results:
    for i, val in enumerate(values):
        x_computed[start_idx + i] = val

print("\nComplete solution vector:")
for i in range(n):
    print(f"x[{i}] = {x_computed[i]:.10f}")

# Compare with true solution if available
try:
    with open('axb_test/solution_reference.txt', 'r') as f:
        lines = f.readlines()
        n_ref = int(lines[1])
        x_true = np.array([float(lines[i+2].strip()) for i in range(n_ref)])

    print("\nTrue solution vector:")
    for i in range(n):
        print(f"x[{i}] = {x_true[i]:.10f}")

    error = np.abs(x_computed - x_true)
    rel_error = error / (np.abs(x_true) + 1e-10)

    print("\nError analysis:")
    print(f"Max absolute error: {np.max(error):.10e}")
    print(f"Mean absolute error: {np.mean(error):.10e}")
    print(f"Max relative error: {np.max(rel_error):.10e}")
    print(f"Mean relative error: {np.mean(rel_error):.10e}")

    # Verify Ax = b
    with open('axb_test/matrix.txt', 'r') as f:
        lines = f.readlines()
        n_mat = int(lines[0])
        A = np.zeros((n_mat, n_mat))
        for i in range(n_mat):
            A[i, :] = list(map(float, lines[i+1].split()))
        b = np.array([float(lines[n_mat+1+i].strip()) for i in range(n_mat)])

    residual = A @ x_computed - b
    print(f"\nResidual ||Ax - b||: {np.linalg.norm(residual):.10e}")
    print(f"Relative residual: {np.linalg.norm(residual) / np.linalg.norm(b):.10e}")

except FileNotFoundError:
    print("\nReference solution not found")

PYTHON_SCRIPT

echo
echo "======================================"
echo "Test completed successfully!"
echo "======================================"
