# Ax=b Monte Carlo Solver - BOINC Tutorial

This example demonstrates how to solve linear systems **Ax = b** using the **Ulam-von Neumann Monte Carlo method**, distributed across multiple BOINC clients.

## Why This is a Good BOINC Example

The Monte Carlo method for solving linear systems is **naturally parallelizable**:

1. **Independent Components**: Different solution vector components can be computed independently
2. **Distributed Work**: Each BOINC client computes a subset of components
3. **Fault Tolerance**: If one work unit fails, others can complete independently
4. **Scalability**: Easy to add more work units for larger systems or higher accuracy
5. **Result Merging**: The server validator merges partial results into complete solution

## The Ulam-von Neumann Method

### Mathematical Background

For a system **Ax = b**, we convert it to the form:
```
x = Cx + f
```

where:
- **C = I - D⁻¹A** (iteration matrix, D is diagonal of A)
- **f = D⁻¹b**

For each component **x_i**, we estimate it by simulating random walks:
1. Start at state i
2. At state j, add **f_j** to the sum
3. Transition to next state k with probability **|C_jk| / Σ|C_jl|**
4. Track the sign of C_jk in the weight
5. Terminate randomly and average over many walks

### Convergence Requirements

The method converges when the spectral radius **ρ(C) < 1**. This is guaranteed when:
- **A is diagonally dominant**: |A_ii| > Σ(j≠i) |A_ij|
- Or A is symmetric positive definite

## Files

### Client Application
- **`src/Axb-MonteCarlo.c`**: Monte Carlo solver (BOINC client application)
  - Reads matrix A, vector b, and component range to compute
  - Performs random walks to estimate solution components
  - Outputs partial solution for specified components

### Server Components
- **`server/axb_validator.cpp`**: Validator that merges partial solutions
  - Collects results from multiple work units
  - Verifies consistency between overlapping components
  - Checks for complete coverage of solution vector
  - Grants credits based on computational work

### Tools
- **`tools/generate_axb_work.py`**: Work generator
  - Creates test matrices (diagonally dominant for convergence)
  - Splits solution vector into multiple work units
  - Distributes components across BOINC clients
  - Generates input files for each work unit

### Testing
- **`examples/test_axb_montecarlo.sh`**: Standalone test script
  - Generates test matrix
  - Runs multiple work units locally
  - Merges results and verifies accuracy

## Usage

### Standalone Testing (Without BOINC)

```bash
cd examples
./test_axb_montecarlo.sh
```

This will:
1. Generate a 5×5 test matrix
2. Split it into 3 work units
3. Run each work unit with 50,000 random walks per component
4. Merge the partial results
5. Compare with the true solution

### With BOINC Server

#### 1. Compile the Client Application

```bash
cd src
gcc -D_BOINC_ -o axb_montecarlo Axb-MonteCarlo.c -lm \
    -I/path/to/boinc/api -I/path/to/boinc/lib \
    -L/path/to/boinc/api -L/path/to/boinc/lib \
    -lboinc_api -lboinc
```

#### 2. Compile the Validator

```bash
cd server
g++ -o axb_validator axb_validator.cpp \
    -I/path/to/boinc/sched -I/path/to/boinc/lib \
    -L/path/to/boinc/sched -L/path/to/boinc/lib \
    -lsched -lboinc
```

#### 3. Generate Work Units

```bash
cd tools
./generate_axb_work.py \
    --dimension 100 \
    --num-work-units 10 \
    --num-walks 1000000 \
    --boinc-project-dir /path/to/boinc/project
```

This creates 10 work units, each computing ~10 components of a 100-dimensional system.

#### 4. Deploy to BOINC Project

Copy files to your BOINC project:

```bash
# Client application
cp src/axb_montecarlo $BOINC_PROJECT/apps/axb_montecarlo/1.0/

# Validator
cp server/axb_validator $BOINC_PROJECT/bin/

# Create XML templates (see templates/ directory)
cp templates/axb_in.xml $BOINC_PROJECT/templates/
cp templates/axb_out.xml $BOINC_PROJECT/templates/
```

## Work Unit Structure

### Input Format
Each work unit receives:
```
n                           # Matrix dimension
A[0][0] A[0][1] ... A[0][n-1]  # Matrix A (row by row)
...
A[n-1][0] ... A[n-1][n-1]
b[0]                        # Vector b
...
b[n-1]
start_idx end_idx num_walks  # Work unit parameters
```

### Output Format
Each work unit produces:
```
start_idx end_idx           # Component range computed
x[start_idx]                # Solution values
...
x[end_idx]
```

## Example: Distributing a 100×100 System

For a 100-dimensional system with 10 work units:

| Work Unit | Components | Number of Walks |
|-----------|------------|-----------------|
| WU 0      | 0-9        | 1,000,000       |
| WU 1      | 10-19      | 1,000,000       |
| WU 2      | 20-29      | 1,000,000       |
| ...       | ...        | ...             |
| WU 9      | 90-99      | 1,000,000       |

The server validator:
1. Waits for all 10 work units to complete
2. Verifies consistency (if components overlap)
3. Checks complete coverage (all components 0-99)
4. Grants credits proportional to work done
5. Marks workunit as valid

## Advantages for Students

This example teaches:

1. **Problem Decomposition**: How to split a mathematical problem into independent tasks
2. **Stochastic Algorithms**: Monte Carlo methods and convergence
3. **Distributed Computing**: Coordinating multiple workers
4. **Result Aggregation**: Merging partial results on the server
5. **Error Handling**: Dealing with incomplete or inconsistent results
6. **Scalability**: How to handle larger problems with more work units

## Performance Considerations

### Accuracy vs. Computation
- More random walks → better accuracy but longer runtime
- Typical: 10⁴ - 10⁶ walks per component
- For n-digit accuracy, need ~10²ⁿ walks

### Work Distribution
- Balance components per work unit
- Too few work units → poor parallelization
- Too many work units → overhead dominates
- Typical: 5-20 components per work unit

### Convergence
- Check diagonal dominance before distributing work
- Poorly conditioned systems may need very long walks
- Consider preconditioning for difficult systems

## Extensions

Students can extend this example:

1. **Overlap for Redundancy**: Compute some components in multiple work units for validation
2. **Adaptive Walks**: Vary walk length based on convergence
3. **Preconditioning**: Improve convergence for difficult systems
4. **Different Splitting**: Try column-based or block-based distribution
5. **Sparse Matrices**: Optimize for sparse systems
6. **Iterative Refinement**: Use results as initial guess for deterministic solver

## References

- Ulam, S. and von Neumann, J. (1947). On combination of stochastic and deterministic processes
- Forsythe, G.E. and Leibler, R.A. (1950). Matrix inversion by a Monte Carlo method
- Halton, J.H. (1970). A retrospective and prospective survey of the Monte Carlo method

## License

Licensed under GNU General Public License v3.0

---

For more BOINC examples, see the main [README.md](../README.md)
