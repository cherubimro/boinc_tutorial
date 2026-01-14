# Example Files

This directory contains example input and output files for the PI computation application.

## Files

### example_input.txt
Example input file containing the number of iterations for PI computation.

**Format:** Single integer on one line
**Example:** `1000000000` (1 billion iterations)

**Usage with standalone application:**
```bash
./pi_compute < example_input.txt > my_output.txt
```

**Common iteration counts:**
- `1000000` (1M) - Quick test, ~3.14 accuracy, ~0.1 seconds
- `10000000` (10M) - Medium test, ~3.141 accuracy, ~1 second
- `100000000` (100M) - Good accuracy, ~3.1415 accuracy, ~10 seconds
- `1000000000` (1B) - High accuracy, ~3.14159 accuracy, ~60 seconds

### example_output.txt
Example output showing the format and expected results.

### app_config.xml
Optional client-side application configuration.

### run_example.sh
Interactive script to test the PI application locally before deploying to BOINC.

**Format:**
- Title and separator
- Total iterations processed
- Estimated value of PI
- Error from actual PI (3.141592653589793)
- Accuracy percentage

**Note:** Your actual output will have different PI values due to random sampling.

## Testing Locally

You can test the PI computation application locally before deploying to BOINC.

### Using run_example.sh (Recommended)

The easiest way to test:

```bash
cd ../src
../examples/run_example.sh
```

This interactive script:
- Builds the application if needed
- Presents menu of iteration counts
- Times the computation
- Shows results and checkpoint info
- Displays application messages

**Example session:**
```
BOINC PI Computation - Example Runner
======================================

Select iteration count:
1) Quick test (1 million - ~0.1 seconds)
2) Fast (10 million - ~1 second)
3) Accurate (100 million - ~10 seconds)
4) Very accurate (1 billion - ~100 seconds)
5) Custom

Enter choice [1-5]: 2

Running computation with 10000000 iterations...
Please wait...

===============================================
Computation completed in 1 seconds
===============================================

PI Computation Results
======================
Total iterations: 10000000
Estimated value of PI: 3.141234567890123
...
```

### Manual Testing

```bash
# Compile the application
cd ../src
make

# Run with example input
echo "10000000" | ./pi_compute

# Or from file
./pi_compute < ../examples/example_input.txt

# Save output
echo "100000000" > my_input.txt
./pi_compute < my_input.txt > my_output.txt
cat my_output.txt
```

## Creating BOINC Work Units

When you create BOINC work units, the input file is uploaded to the server and distributed to clients:

```bash
# Create input file for work unit
echo "10000000" > ~/boinc_test/projects/download/wu_10m.txt

# Create work unit
cd ~/boinc_test/projects
bin/create_work --appname pi_compute \
    --wu_name pi_wu_test \
    --wu_template templates/pi_in.xml \
    --result_template templates/pi_out.xml \
    --target_nresults 3 \
    wu_10m.txt
```

The clients will:
1. Download `wu_10m.txt`
2. Run `pi_compute` with the input
3. Upload the result file
4. Server validates results by comparing 3 different client outputs

## Client Configuration (app_config.xml)

This optional file can be placed in the project directory on the **client side** to configure application behavior.

**Location on client:** `~/boinc_clients/client1/app_config.xml`

**What it does:**
```xml
<app_config>
    <app>
        <name>pi_compute</name>
        <max_concurrent>1</max_concurrent>  <!-- Use only 1 CPU core -->
    </app>
</app_config>
```

**Use cases:**
- Limit CPU usage (max_concurrent)
- Set GPU preferences
- Configure application-specific parameters

**Note:** This is **not** needed for the tutorial - it's provided as an example for advanced users who want to control resource usage.

## Expected Behavior

### Single Run (Standalone)
Always produces slightly different results because of `/dev/urandom` seeding.

### BOINC Multi-Client
Each of the 3 clients produces a different PI value, proving:
- Random seeding works correctly
- Cross-validation is functioning
- No deterministic errors

Example from actual BOINC run:
```
Work Unit: pi_wu_17 (10M iterations)
  Client A: PI = 3.141630400 (99.9988% accuracy)
  Client B: PI = 3.141109600 (99.9846% accuracy)
  Client C: PI = 3.141212000 (99.9879% accuracy)

Status: ✓ All 3 clients produced different results
        ✓ Cross-validation working correctly
        ✓ Random seeding functioning properly
```

## Modifying for Different Computations

To compute something else (e.g., value of e, Fibonacci):

1. Modify `pi_compute.cpp` computation logic
2. Change input format if needed
3. Update output formatting
4. Recompile with `make`
5. Redeploy to BOINC with `bin/update_versions`

See README.md for extension ideas and exercises.
