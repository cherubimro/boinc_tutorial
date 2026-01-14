/*
 * simpleAxbMC.c
 *
 * Standalone implementation of Ulam-von Neumann Monte Carlo method
 * for solving linear systems Ax = b
 *
 * Pure C implementation without BOINC dependencies
 * Compares Monte Carlo solution against direct Gaussian elimination
 *
 * Compile: gcc -o simpleAxbMC simpleAxbMC.c -lm
 * Usage: ./simpleAxbMC [dimension] [num_walks]
 *
 * Licensed under GPL v3
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define MAX_DIM 100
#define DEFAULT_WALKS 100000
#define MAX_WALK_LENGTH 10000
#define TERMINATION_PROB 0.1

typedef struct {
    int n;                    // Dimension
    double A[MAX_DIM][MAX_DIM]; // Original matrix
    double b[MAX_DIM];         // Right-hand side
    double C[MAX_DIM][MAX_DIM]; // Iteration matrix C = I - D^{-1}A
    double f[MAX_DIM];         // f = D^{-1}b
    double row_sum[MAX_DIM];   // Sum of |C_ij| for transition probabilities
    double x_mc[MAX_DIM];      // Monte Carlo solution
    double x_direct[MAX_DIM];  // Direct solution (Gaussian elimination)
} LinearSystem;

// Initialize random number generator with high-quality seed
void init_random() {
    unsigned int seed;
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (urandom) {
        fread(&seed, sizeof(seed), 1, urandom);
        fclose(urandom);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    printf("Random seed: %u\n\n", seed);
}

// Generate random double in [0, 1)
double rand_double() {
    return (double)rand() / (RAND_MAX + 1.0);
}

// Generate a random diagonally dominant matrix (ensures convergence)
void generate_diagonal_dominant_matrix(LinearSystem* sys) {
    printf("Generating %dx%d diagonally dominant system...\n", sys->n, sys->n);

    // Generate off-diagonal elements
    for (int i = 0; i < sys->n; i++) {
        double row_sum = 0.0;
        for (int j = 0; j < sys->n; j++) {
            if (i != j) {
                sys->A[i][j] = (rand_double() - 0.5) * 2.0; // Range [-1, 1]
                row_sum += fabs(sys->A[i][j]);
            }
        }
        // Make diagonal dominant: |A_ii| > sum of |A_ij| for j != i
        sys->A[i][i] = row_sum * 1.5 + 5.0; // Ensures diagonal dominance
    }

    // Generate random true solution
    double x_true[MAX_DIM];
    for (int i = 0; i < sys->n; i++) {
        x_true[i] = (rand_double() - 0.5) * 10.0;
    }

    // Compute b = Ax_true
    for (int i = 0; i < sys->n; i++) {
        sys->b[i] = 0.0;
        for (int j = 0; j < sys->n; j++) {
            sys->b[i] += sys->A[i][j] * x_true[j];
        }
    }

    printf("True solution (for verification):\n");
    for (int i = 0; i < sys->n; i++) {
        printf("  x[%d] = %.6f\n", i, x_true[i]);
    }
    printf("\n");
}

// Print matrix (for debugging)
void print_matrix(const char* name, int n, double A[MAX_DIM][MAX_DIM]) {
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%8.4f ", A[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Print vector
void print_vector(const char* name, int n, double* v) {
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        printf("  [%d] = %.10f\n", i, v[i]);
    }
    printf("\n");
}

// Prepare iteration form: C = I - D^{-1}A, f = D^{-1}b
int prepare_iteration_form(LinearSystem* sys) {
    printf("Preparing iteration form (C = I - D^{-1}A, f = D^{-1}b)...\n");

    for (int i = 0; i < sys->n; i++) {
        double diag = sys->A[i][i];

        if (fabs(diag) < 1e-12) {
            fprintf(stderr, "Error: Zero diagonal element A[%d][%d] = %g\n", i, i, diag);
            return -1;
        }

        // f_i = b_i / A_ii
        sys->f[i] = sys->b[i] / diag;

        // C_ij = delta_ij - A_ij/A_ii
        sys->row_sum[i] = 0.0;
        for (int j = 0; j < sys->n; j++) {
            if (i == j) {
                sys->C[i][j] = 0.0;
            } else {
                sys->C[i][j] = -sys->A[i][j] / diag;
            }
            sys->row_sum[i] += fabs(sys->C[i][j]);
        }

        printf("  Row %d: diagonal = %.4f, row_sum = %.4f\n", i, diag, sys->row_sum[i]);
    }

    // Check convergence condition
    double max_row_sum = 0.0;
    for (int i = 0; i < sys->n; i++) {
        if (sys->row_sum[i] > max_row_sum) {
            max_row_sum = sys->row_sum[i];
        }
    }

    printf("Max row sum of |C|: %.6f (should be < 1 for convergence)\n", max_row_sum);
    if (max_row_sum >= 1.0) {
        printf("Warning: Convergence not guaranteed!\n");
    }
    printf("\n");

    return 0;
}

// Perform one random walk starting from state i
double random_walk(LinearSystem* sys, int start_state) {
    double sum = 0.0;
    int current_state = start_state;
    double weight = 1.0;

    for (int step = 0; step < MAX_WALK_LENGTH; step++) {
        // Add contribution from current state
        sum += weight * sys->f[current_state];

        // Terminate with probability TERMINATION_PROB
        if (rand_double() < TERMINATION_PROB) {
            break;
        }

        // No transitions available
        if (sys->row_sum[current_state] < 1e-12) {
            break;
        }

        // Choose next state based on transition probabilities
        double r = rand_double() * sys->row_sum[current_state];
        double cumsum = 0.0;
        int next_state = current_state;

        for (int j = 0; j < sys->n; j++) {
            cumsum += fabs(sys->C[current_state][j]);
            if (r <= cumsum) {
                next_state = j;
                // Update weight with sign of C_ij and normalization
                weight *= (sys->C[current_state][j] >= 0) ? 1.0 : -1.0;
                weight *= sys->row_sum[current_state] / (1.0 - TERMINATION_PROB);
                break;
            }
        }

        current_state = next_state;
    }

    return sum;
}

// Solve using Monte Carlo method
void solve_monte_carlo(LinearSystem* sys, long num_walks) {
    printf("Solving with Monte Carlo (%ld walks per component)...\n", num_walks);

    clock_t start = clock();

    for (int i = 0; i < sys->n; i++) {
        double sum = 0.0;

        for (long walk = 0; walk < num_walks; walk++) {
            sum += random_walk(sys, i);
        }

        sys->x_mc[i] = sum / num_walks;

        if ((i + 1) % 10 == 0 || i == sys->n - 1) {
            printf("  Computed %d/%d components\n", i + 1, sys->n);
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Monte Carlo solution completed in %.3f seconds\n\n", time_spent);
}

// Gaussian elimination with partial pivoting
void solve_gaussian_elimination(LinearSystem* sys) {
    printf("Solving with Gaussian elimination...\n");

    clock_t start = clock();

    // Make working copies
    double A_work[MAX_DIM][MAX_DIM];
    double b_work[MAX_DIM];

    memcpy(A_work, sys->A, sizeof(A_work));
    memcpy(b_work, sys->b, sizeof(b_work));

    int n = sys->n;

    // Forward elimination with partial pivoting
    for (int k = 0; k < n - 1; k++) {
        // Find pivot
        int pivot_row = k;
        double max_val = fabs(A_work[k][k]);

        for (int i = k + 1; i < n; i++) {
            if (fabs(A_work[i][k]) > max_val) {
                max_val = fabs(A_work[i][k]);
                pivot_row = i;
            }
        }

        // Swap rows if necessary
        if (pivot_row != k) {
            for (int j = 0; j < n; j++) {
                double temp = A_work[k][j];
                A_work[k][j] = A_work[pivot_row][j];
                A_work[pivot_row][j] = temp;
            }
            double temp = b_work[k];
            b_work[k] = b_work[pivot_row];
            b_work[pivot_row] = temp;
        }

        // Eliminate column
        for (int i = k + 1; i < n; i++) {
            double factor = A_work[i][k] / A_work[k][k];
            for (int j = k; j < n; j++) {
                A_work[i][j] -= factor * A_work[k][j];
            }
            b_work[i] -= factor * b_work[k];
        }
    }

    // Back substitution
    for (int i = n - 1; i >= 0; i--) {
        sys->x_direct[i] = b_work[i];
        for (int j = i + 1; j < n; j++) {
            sys->x_direct[i] -= A_work[i][j] * sys->x_direct[j];
        }
        sys->x_direct[i] /= A_work[i][i];
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Gaussian elimination completed in %.6f seconds\n\n", time_spent);
}

// Compare solutions and compute errors
void compare_solutions(LinearSystem* sys) {
    printf("========================================\n");
    printf("SOLUTION COMPARISON\n");
    printf("========================================\n\n");

    printf("%-5s %15s %15s %15s %15s\n", "i", "Monte Carlo", "Direct", "Abs Error", "Rel Error");
    printf("--------------------------------------------------------------------------------\n");

    double max_abs_error = 0.0;
    double max_rel_error = 0.0;
    double sum_abs_error = 0.0;
    double sum_rel_error = 0.0;

    for (int i = 0; i < sys->n; i++) {
        double abs_error = fabs(sys->x_mc[i] - sys->x_direct[i]);
        double rel_error = abs_error / (fabs(sys->x_direct[i]) + 1e-10);

        printf("%-5d %15.10f %15.10f %15.10e %15.10e\n",
               i, sys->x_mc[i], sys->x_direct[i], abs_error, rel_error);

        if (abs_error > max_abs_error) max_abs_error = abs_error;
        if (rel_error > max_rel_error) max_rel_error = rel_error;
        sum_abs_error += abs_error;
        sum_rel_error += rel_error;
    }

    printf("\n");
    printf("Error Statistics:\n");
    printf("  Max absolute error:  %.10e\n", max_abs_error);
    printf("  Mean absolute error: %.10e\n", sum_abs_error / sys->n);
    printf("  Max relative error:  %.10e (%.6f%%)\n", max_rel_error, max_rel_error * 100);
    printf("  Mean relative error: %.10e (%.6f%%)\n",
           sum_rel_error / sys->n, (sum_rel_error / sys->n) * 100);
    printf("\n");
}

// Verify solution by computing residual ||Ax - b||
void verify_solution(LinearSystem* sys, const char* name, double* x) {
    double residual[MAX_DIM];
    double max_residual = 0.0;
    double norm_b = 0.0;

    for (int i = 0; i < sys->n; i++) {
        residual[i] = -sys->b[i];
        for (int j = 0; j < sys->n; j++) {
            residual[i] += sys->A[i][j] * x[j];
        }
        max_residual += residual[i] * residual[i];
        norm_b += sys->b[i] * sys->b[i];
    }

    max_residual = sqrt(max_residual);
    norm_b = sqrt(norm_b);

    printf("%s residual verification:\n", name);
    printf("  ||Ax - b||:           %.10e\n", max_residual);
    printf("  ||b||:                %.10e\n", norm_b);
    printf("  Relative residual:    %.10e\n", max_residual / norm_b);
    printf("\n");
}

int main(int argc, char** argv) {
    LinearSystem sys;
    int dimension = 5;
    long num_walks = DEFAULT_WALKS;

    // Parse command line arguments
    if (argc > 1) {
        dimension = atoi(argv[1]);
        if (dimension < 1 || dimension > MAX_DIM) {
            fprintf(stderr, "Error: Dimension must be between 1 and %d\n", MAX_DIM);
            return 1;
        }
    }

    if (argc > 2) {
        num_walks = atol(argv[2]);
        if (num_walks < 1) {
            fprintf(stderr, "Error: Number of walks must be positive\n");
            return 1;
        }
    }

    sys.n = dimension;

    printf("========================================\n");
    printf("Ulam-von Neumann Monte Carlo Solver\n");
    printf("========================================\n");
    printf("Dimension:     %d x %d\n", dimension, dimension);
    printf("Walks/component: %ld\n\n", num_walks);

    // Initialize random number generator
    init_random();

    // Generate test problem
    generate_diagonal_dominant_matrix(&sys);

    // Prepare iteration form
    if (prepare_iteration_form(&sys) < 0) {
        return 1;
    }

    // Solve with Monte Carlo
    solve_monte_carlo(&sys, num_walks);

    // Solve with direct method
    solve_gaussian_elimination(&sys);

    // Compare solutions
    compare_solutions(&sys);

    // Verify both solutions
    verify_solution(&sys, "Monte Carlo", sys.x_mc);
    verify_solution(&sys, "Direct", sys.x_direct);

    printf("========================================\n");
    printf("Recommendations:\n");
    printf("========================================\n");
    printf("For better accuracy:\n");
    printf("  - Increase number of walks (current: %ld)\n", num_walks);
    printf("  - Try: %s %d %ld\n", argv[0], dimension, num_walks * 10);
    printf("\nFor larger systems:\n");
    printf("  - Try: %s 20 100000\n", argv[0]);
    printf("  - Try: %s 50 1000000\n", argv[0]);
    printf("\n");

    return 0;
}
