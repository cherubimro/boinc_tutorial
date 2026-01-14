#!/usr/bin/env python3
"""
BOINC Work Generator for PI Computation Project

This script generates work units for the PI computation application.
It creates input files with varying iteration counts and submits them
as work units to the BOINC server.

Usage:
    ./generate_work.py --num_wu 100 --iterations 100000000
    ./generate_work.py --range 10000000:1000000000:10
"""

import os
import sys
import argparse
import subprocess
import time
from pathlib import Path


class PIWorkGenerator:
    def __init__(self, project_dir):
        """
        Initialize the work generator.

        Args:
            project_dir: Path to BOINC project directory (e.g., ~/projects/pi_compute)
        """
        self.project_dir = Path(project_dir)
        self.download_dir = self.project_dir / "download"
        self.templates_dir = self.project_dir / "templates"
        self.bin_dir = self.project_dir / "bin"

        # Verify directories exist
        if not self.project_dir.exists():
            raise FileNotFoundError(f"Project directory not found: {project_dir}")

        self.download_dir.mkdir(exist_ok=True)

    def create_input_file(self, iterations, filename):
        """
        Create an input file with specified iteration count.

        Args:
            iterations: Number of iterations for PI computation
            filename: Name of the input file to create

        Returns:
            Path to created file
        """
        filepath = self.download_dir / filename
        with open(filepath, 'w') as f:
            f.write(f"{iterations}\n")

        print(f"Created input file: {filename} ({iterations} iterations)")
        return filepath

    def create_work_unit(self, wu_name, input_file, fpops_est=1e12):
        """
        Create a BOINC work unit.

        Args:
            wu_name: Unique name for the work unit
            input_file: Path to input file
            fpops_est: Estimated floating point operations

        Returns:
            True if successful, False otherwise
        """
        create_work_cmd = [
            str(self.bin_dir / "create_work"),
            "--appname", "pi_compute",
            "--wu_name", wu_name,
            "--wu_template", str(self.templates_dir / "pi_in.xml"),
            "--result_template", str(self.templates_dir / "pi_out.xml"),
            "--rsc_fpops_est", str(int(fpops_est)),
            "--rsc_fpops_bound", str(int(fpops_est * 10)),
            "--rsc_memory_bound", "100000000",
            "--rsc_disk_bound", "100000000",
            "--delay_bound", "86400",  # 24 hours
            "--min_quorum", "2",  # Need 2 results
            "--target_nresults", "2",
            str(input_file)
        ]

        try:
            result = subprocess.run(
                create_work_cmd,
                cwd=str(self.project_dir),
                capture_output=True,
                text=True,
                check=True
            )
            print(f"✓ Created work unit: {wu_name}")
            return True
        except subprocess.CalledProcessError as e:
            print(f"✗ Failed to create work unit {wu_name}:")
            print(f"  Error: {e.stderr}")
            return False

    def generate_fixed_work(self, num_units, iterations):
        """
        Generate multiple work units with same iteration count.

        Args:
            num_units: Number of work units to create
            iterations: Iterations per work unit
        """
        print(f"\nGenerating {num_units} work units with {iterations} iterations each...")
        print("=" * 70)

        success_count = 0
        for i in range(num_units):
            wu_name = f"pi_wu_{iterations}_{i:06d}"
            input_filename = f"pi_in_{wu_name}.txt"

            # Create input file
            input_file = self.create_input_file(iterations, input_filename)

            # Estimate FLOPs based on iterations
            fpops_est = iterations * 10  # Rough estimate

            # Create work unit
            if self.create_work_unit(wu_name, input_file, fpops_est):
                success_count += 1

            # Small delay to avoid overwhelming the database
            if i < num_units - 1:
                time.sleep(0.1)

        print("=" * 70)
        print(f"Summary: {success_count}/{num_units} work units created successfully")

    def generate_range_work(self, min_iter, max_iter, num_units):
        """
        Generate work units with varying iteration counts.

        Args:
            min_iter: Minimum iterations
            max_iter: Maximum iterations
            num_units: Number of work units to create
        """
        print(f"\nGenerating {num_units} work units with iterations from {min_iter} to {max_iter}...")
        print("=" * 70)

        # Calculate iteration step
        if num_units == 1:
            iterations_list = [max_iter]
        else:
            step = (max_iter - min_iter) / (num_units - 1)
            iterations_list = [int(min_iter + i * step) for i in range(num_units)]

        success_count = 0
        for i, iterations in enumerate(iterations_list):
            wu_name = f"pi_wu_var_{i:06d}"
            input_filename = f"pi_in_{wu_name}.txt"

            # Create input file
            input_file = self.create_input_file(iterations, input_filename)

            # Estimate FLOPs
            fpops_est = iterations * 10

            # Create work unit
            if self.create_work_unit(wu_name, input_file, fpops_est):
                success_count += 1

            if i < num_units - 1:
                time.sleep(0.1)

        print("=" * 70)
        print(f"Summary: {success_count}/{num_units} work units created successfully")


def main():
    parser = argparse.ArgumentParser(
        description="Generate work units for PI computation BOINC project",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate 100 work units with 100 million iterations each
  %(prog)s --num_wu 100 --iterations 100000000

  # Generate 50 work units with varying iterations from 10M to 1B
  %(prog)s --range 10000000:1000000000:50

  # Use custom project directory
  %(prog)s --project_dir ~/projects/pi_compute --num_wu 10 --iterations 50000000
        """
    )

    parser.add_argument(
        '--project_dir',
        default=os.path.expanduser('~/projects/pi_compute'),
        help='Path to BOINC project directory (default: ~/projects/pi_compute)'
    )

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '--num_wu',
        type=int,
        help='Number of work units to generate (requires --iterations)'
    )
    group.add_argument(
        '--range',
        help='Generate work units with varying iterations (format: MIN:MAX:COUNT)'
    )

    parser.add_argument(
        '--iterations',
        type=int,
        help='Number of iterations per work unit (used with --num_wu)'
    )

    args = parser.parse_args()

    # Validate arguments
    if args.num_wu and not args.iterations:
        parser.error("--num_wu requires --iterations")

    try:
        generator = PIWorkGenerator(args.project_dir)

        if args.num_wu:
            generator.generate_fixed_work(args.num_wu, args.iterations)
        elif args.range:
            try:
                min_iter, max_iter, count = map(int, args.range.split(':'))
                generator.generate_range_work(min_iter, max_iter, count)
            except ValueError:
                parser.error("--range must be in format MIN:MAX:COUNT (e.g., 10000000:1000000000:50)")

        print("\n✓ Work generation complete!")
        print("\nNext steps:")
        print("  1. Check work units: cd ~/projects/pi_compute && bin/db_query 'SELECT COUNT(*) FROM workunit'")
        print("  2. Ensure daemons are running: bin/status")
        print("  3. Monitor progress: tail -f log_*/scheduler.log")

    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
