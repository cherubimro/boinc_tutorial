#!/usr/bin/env python3
"""
generate_axb_work.py

Generates BOINC work units for the Ax=b Monte Carlo solver.
Splits the solution vector into multiple work units, where each work unit
computes a subset of the solution components.

This demonstrates how to parallelize a naturally divisible problem across
multiple BOINC clients.

Licensed under GPL v3
"""

import sys
import os
import argparse
import numpy as np
import subprocess

def generate_test_matrix(n, condition_number=10.0, diagonal_dominant=True):
    """
    Generate a test matrix A and vector b for the system Ax = b

    Args:
        n: Matrix dimension
        condition_number: Desired condition number (controls difficulty)
        diagonal_dominant: If True, ensures diagonal dominance for convergence

    Returns:
        A: n x n matrix
        b: n vector
        x_true: True solution (for verification)
    """
    # Generate random true solution
    x_true = np.random.randn(n)

    if diagonal_dominant:
        # Create diagonally dominant matrix for guaranteed convergence
        A = np.random.randn(n, n) * 0.1  # Off-diagonal elements small

        # Make diagonal dominant
        for i in range(n):
            row_sum = np.sum(np.abs(A[i, :])) - np.abs(A[i, i])
            A[i, i] = row_sum * 1.5 + 1.0  # Diagonal dominates
    else:
        # Generate random well-conditioned matrix
        U, _, V = np.linalg.svd(np.random.randn(n, n))
        # Singular values from 1 to condition_number
        S = np.diag(np.linspace(1, condition_number, n))
        A = U @ S @ V

    # Compute b from true solution
    b = A @ x_true

    return A, b, x_true


def write_input_file(filename, A, b, start_idx, end_idx, num_walks):
    """
    Write input file for one work unit

    Format:
        n (dimension)
        A (matrix, row by row)
        b (vector)
        start_idx end_idx num_walks (work unit parameters)
    """
    n = len(b)

    with open(filename, 'w') as f:
        # Write dimension
        f.write(f"{n}\n")

        # Write matrix A
        for i in range(n):
            for j in range(n):
                f.write(f"{A[i, j]:.15e} ")
            f.write("\n")

        # Write vector b
        for i in range(n):
            f.write(f"{b[i]:.15e}\n")

        # Write work unit parameters
        f.write(f"{start_idx} {end_idx} {num_walks}\n")


def create_work_unit(work_dir, wu_name, input_file, app_name="axb_montecarlo"):
    """
    Create a BOINC work unit using the create_work tool

    Args:
        work_dir: BOINC project directory
        wu_name: Work unit name
        input_file: Path to input file
        app_name: BOINC application name
    """
    cmd = [
        os.path.join(work_dir, "bin", "create_work"),
        "--appname", app_name,
        "--wu_name", wu_name,
        "--wu_template", os.path.join(work_dir, "templates", "axb_in.xml"),
        "--result_template", os.path.join(work_dir, "templates", "axb_out.xml"),
        input_file
    ]

    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"Created work unit: {wu_name}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error creating work unit {wu_name}: {e.stderr}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Generate BOINC work units for Ax=b Monte Carlo solver"
    )

    parser.add_argument(
        "-n", "--dimension",
        type=int,
        default=10,
        help="Matrix dimension (default: 10)"
    )

    parser.add_argument(
        "-w", "--num-work-units",
        type=int,
        default=5,
        help="Number of work units to create (default: 5)"
    )

    parser.add_argument(
        "-s", "--num-walks",
        type=int,
        default=100000,
        help="Number of Monte Carlo walks per component (default: 100000)"
    )

    parser.add_argument(
        "-o", "--output-dir",
        type=str,
        default="./wu_inputs",
        help="Directory for work unit input files (default: ./wu_inputs)"
    )

    parser.add_argument(
        "--boinc-project-dir",
        type=str,
        default=None,
        help="BOINC project directory (if set, will create work units)"
    )

    parser.add_argument(
        "--matrix-file",
        type=str,
        default=None,
        help="Use existing matrix from file (format: n, A, b)"
    )

    parser.add_argument(
        "--diagonal-dominant",
        action="store_true",
        default=True,
        help="Generate diagonally dominant matrix (ensures convergence)"
    )

    parser.add_argument(
        "--save-matrix",
        type=str,
        default=None,
        help="Save generated matrix to file for later use"
    )

    args = parser.parse_args()

    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)

    # Generate or load matrix
    if args.matrix_file:
        print(f"Loading matrix from {args.matrix_file}...")
        data = np.loadtxt(args.matrix_file)
        n = int(data[0])
        A = data[1:n+1, :].reshape(n, n)
        b = data[n+1, :]
        x_true = None
    else:
        print(f"Generating {args.dimension}x{args.dimension} test matrix...")
        A, b, x_true = generate_test_matrix(
            args.dimension,
            diagonal_dominant=args.diagonal_dominant
        )
        n = args.dimension

        # Save matrix if requested
        if args.save_matrix:
            with open(args.save_matrix, 'w') as f:
                f.write(f"{n}\n")
                for i in range(n):
                    for j in range(n):
                        f.write(f"{A[i, j]:.15e} ")
                    f.write("\n")
                for i in range(n):
                    f.write(f"{b[i]:.15e}\n")
            print(f"Matrix saved to {args.save_matrix}")

    # Print matrix properties
    print(f"\nMatrix properties:")
    print(f"  Dimension: {n}x{n}")
    print(f"  Condition number: {np.linalg.cond(A):.2e}")
    print(f"  Norm of b: {np.linalg.norm(b):.6f}")

    # Check diagonal dominance
    diag_dom = True
    for i in range(n):
        row_sum = np.sum(np.abs(A[i, :])) - np.abs(A[i, i])
        if np.abs(A[i, i]) <= row_sum:
            diag_dom = False
            break
    print(f"  Diagonally dominant: {diag_dom}")

    if x_true is not None:
        print(f"  True solution norm: {np.linalg.norm(x_true):.6f}")

    # Distribute components across work units
    num_wu = args.num_work_units
    components_per_wu = n // num_wu
    remainder = n % num_wu

    print(f"\nDistributing {n} components across {num_wu} work units:")

    wu_ranges = []
    start = 0
    for i in range(num_wu):
        # Give extra components to first 'remainder' work units
        end = start + components_per_wu + (1 if i < remainder else 0) - 1
        wu_ranges.append((start, end))
        start = end + 1

    # Generate work unit input files
    for i, (start_idx, end_idx) in enumerate(wu_ranges):
        wu_name = f"axb_wu_{i:04d}"
        input_file = os.path.join(args.output_dir, f"{wu_name}_input.txt")

        print(f"  WU {i}: components {start_idx}-{end_idx} " +
              f"({end_idx - start_idx + 1} components)")

        write_input_file(input_file, A, b, start_idx, end_idx, args.num_walks)

        # Create BOINC work unit if project directory specified
        if args.boinc_project_dir:
            create_work_unit(args.boinc_project_dir, wu_name, input_file)

    print(f"\nGenerated {num_wu} work unit input files in {args.output_dir}/")

    # Write summary file with true solution for verification
    if x_true is not None:
        summary_file = os.path.join(args.output_dir, "solution_reference.txt")
        with open(summary_file, 'w') as f:
            f.write(f"# True solution for verification\n")
            f.write(f"{n}\n")
            for i in range(n):
                f.write(f"{x_true[i]:.15e}\n")
        print(f"Reference solution saved to {summary_file}")

    print("\nTo create BOINC work units, run:")
    print(f"  {sys.argv[0]} --boinc-project-dir /path/to/boinc/project \\")
    print(f"    -n {args.dimension} -w {args.num_work_units} -s {args.num_walks}")


if __name__ == "__main__":
    main()
