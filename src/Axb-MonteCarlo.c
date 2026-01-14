/*
 * Axb-MonteCarlo.c
 *
 * Solves Ax = b using the Ulam-von Neumann Monte Carlo method
 * This is a naturally parallelizable algorithm - different work units
 * can compute different components of the solution vector x
 *
 * The method works by:
 * 1. Converting the system to the form x = Cx + f where C = I - D^{-1}A and f = D^{-1}b
 * 2. For each component x_i, simulate random walks using the matrix C
 * 3. Each walk contributes to the expected value of x_i
 *
 * BOINC Integration:
 * - Input: Matrix A, vector b, component indices to compute, number of walks
 * - Output: Computed values for specified components
 * - Server merges results from multiple work units to get complete solution
 *
 * Licensed under GPL v3
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef _BOINC_
#include "boinc_api.h"
#include "filesys.h"
#endif

#define MAX_DIM 1000
#define DEFAULT_WALKS 100000

typedef struct {
    int n;                    // Matrix dimension
    double A[MAX_DIM][MAX_DIM]; // Coefficient matrix
    double b[MAX_DIM];         // Right-hand side vector
    double C[MAX_DIM][MAX_DIM]; // Iteration matrix C = I - D^{-1}A
    double f[MAX_DIM];         // f = D^{-1}b
    double row_sum[MAX_DIM];   // Sum of |C_ij| for each row (for transition probabilities)
    int start_idx;             // First component to compute
    int end_idx;               // Last component to compute (inclusive)
    long num_walks;            // Number of random walks per component
} MonteCarloData;

// Initialize random number generator
void init_random() {
    unsigned int seed = time(NULL);
#ifdef _BOINC_
    // Use more varied seed for BOINC work units
    seed ^= (unsigned int)getpid();
#endif
    srand(seed);
}

// Generate random double in [0, 1)
double rand_double() {
    return (double)rand() / (RAND_MAX + 1.0);
}

// Read matrix A and vector b from input file
int read_input(const char *filename, MonteCarloData *data) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open input file %s\n", filename);
        return -1;
    }

    // Read dimension
    if (fscanf(fp, "%d", &data->n) != 1) {
        fprintf(stderr, "Error: Cannot read matrix dimension\n");
        fclose(fp);
        return -1;
    }

    if (data->n <= 0 || data->n > MAX_DIM) {
        fprintf(stderr, "Error: Invalid dimension %d (must be 1-%d)\n", data->n, MAX_DIM);
        fclose(fp);
        return -1;
    }

    // Read matrix A
    for (int i = 0; i < data->n; i++) {
        for (int j = 0; j < data->n; j++) {
            if (fscanf(fp, "%lf", &data->A[i][j]) != 1) {
                fprintf(stderr, "Error: Cannot read matrix element A[%d][%d]\n", i, j);
                fclose(fp);
                return -1;
            }
        }
    }

    // Read vector b
    for (int i = 0; i < data->n; i++) {
        if (fscanf(fp, "%lf", &data->b[i]) != 1) {
            fprintf(stderr, "Error: Cannot read vector element b[%d]\n", i);
            fclose(fp);
            return -1;
        }
    }

    // Read work unit parameters
    if (fscanf(fp, "%d %d %ld", &data->start_idx, &data->end_idx, &data->num_walks) != 3) {
        // Default: compute all components
        data->start_idx = 0;
        data->end_idx = data->n - 1;
        data->num_walks = DEFAULT_WALKS;
    }

    fclose(fp);
    return 0;
}

// Prepare iteration matrix C = I - D^{-1}A and vector f = D^{-1}b
// Using Jacobi iteration form (D is diagonal of A)
int prepare_iteration_form(MonteCarloData *data) {
    for (int i = 0; i < data->n; i++) {
        double diag = data->A[i][i];

        if (fabs(diag) < 1e-12) {
            fprintf(stderr, "Error: Zero diagonal element A[%d][%d] = %g\n", i, i, diag);
            return -1;
        }

        // f_i = b_i / A_ii
        data->f[i] = data->b[i] / diag;

        // C_ij = delta_ij - A_ij/A_ii
        data->row_sum[i] = 0.0;
        for (int j = 0; j < data->n; j++) {
            if (i == j) {
                data->C[i][j] = 0.0;  // 1 - A_ii/A_ii = 0
            } else {
                data->C[i][j] = -data->A[i][j] / diag;
            }
            data->row_sum[i] += fabs(data->C[i][j]);
        }

        // Check convergence condition: row sum should be < 1
        if (data->row_sum[i] >= 1.0) {
            fprintf(stderr, "Warning: Row %d has sum %g >= 1, convergence not guaranteed\n",
                    i, data->row_sum[i]);
        }
    }

    return 0;
}

// Perform one random walk starting from state i
// Returns the sum accumulated along the walk
double random_walk(MonteCarloData *data, int start_state) {
    const int MAX_STEPS = 10000;
    const double TERMINATION_PROB = 0.1;  // Probability to terminate at each step

    double sum = 0.0;
    int current_state = start_state;
    double weight = 1.0;

    for (int step = 0; step < MAX_STEPS; step++) {
        // Add contribution from current state
        sum += weight * data->f[current_state];

        // Terminate with some probability
        if (rand_double() < TERMINATION_PROB) {
            break;
        }

        // Choose next state based on transition probabilities
        // P(i -> j) = |C_ij| / sum_k |C_ik|
        if (data->row_sum[current_state] < 1e-12) {
            break;  // No transitions available
        }

        double r = rand_double() * data->row_sum[current_state];
        double cumsum = 0.0;
        int next_state = current_state;

        for (int j = 0; j < data->n; j++) {
            cumsum += fabs(data->C[current_state][j]);
            if (r <= cumsum) {
                next_state = j;
                // Update weight with sign of C_ij
                weight *= (data->C[current_state][j] >= 0) ? 1.0 : -1.0;
                weight *= data->row_sum[current_state] / (1.0 - TERMINATION_PROB);
                break;
            }
        }

        current_state = next_state;
    }

    return sum;
}

// Compute solution components using Monte Carlo
void compute_solution(MonteCarloData *data, double *x_partial) {
    int num_components = data->end_idx - data->start_idx + 1;

    printf("Computing components %d to %d using %ld walks each\n",
           data->start_idx, data->end_idx, data->num_walks);

    for (int idx = 0; idx < num_components; idx++) {
        int i = data->start_idx + idx;
        double sum = 0.0;

        for (long walk = 0; walk < data->num_walks; walk++) {
            sum += random_walk(data, i);

#ifdef _BOINC_
            // Report progress to BOINC
            if (walk % 1000 == 0) {
                double progress = (idx + (double)walk / data->num_walks) / num_components;
                boinc_fraction_done(progress);
            }
#endif
        }

        // Average over all walks
        x_partial[idx] = sum / data->num_walks;

        printf("x[%d] = %.10f (from %ld walks)\n", i, x_partial[idx], data->num_walks);
    }
}

// Write results to output file
int write_output(const char *filename, MonteCarloData *data, double *x_partial) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create output file %s\n", filename);
        return -1;
    }

    // Write which components were computed
    fprintf(fp, "%d %d\n", data->start_idx, data->end_idx);

    // Write the computed values
    int num_components = data->end_idx - data->start_idx + 1;
    for (int i = 0; i < num_components; i++) {
        fprintf(fp, "%.15e\n", x_partial[i]);
    }

    fclose(fp);
    return 0;
}

// Verify solution (if complete solution is computed)
void verify_solution(MonteCarloData *data, double *x) {
    if (data->start_idx != 0 || data->end_idx != data->n - 1) {
        return;  // Only verify if we computed the complete solution
    }

    printf("\nVerification (computing ||Ax - b||):\n");

    double max_error = 0.0;
    double norm_b = 0.0;

    for (int i = 0; i < data->n; i++) {
        double Ax_i = 0.0;
        for (int j = 0; j < data->n; j++) {
            Ax_i += data->A[i][j] * x[j];
        }
        double error = fabs(Ax_i - data->b[i]);
        if (error > max_error) max_error = error;
        norm_b += data->b[i] * data->b[i];
    }

    norm_b = sqrt(norm_b);
    printf("Max absolute error: %.10e\n", max_error);
    printf("Relative error: %.10e\n", max_error / norm_b);
}

int main(int argc, char **argv) {
    MonteCarloData data;
    double x_partial[MAX_DIM];
    const char *input_file = "input.txt";
    const char *output_file = "output.txt";

#ifdef _BOINC_
    int retval = boinc_init();
    if (retval) {
        fprintf(stderr, "BOINC initialization failed: %d\n", retval);
        exit(retval);
    }

    // Get input/output file paths from BOINC
    char resolved_input[512], resolved_output[512];
    boinc_resolve_filename_s("input.txt", resolved_input, sizeof(resolved_input));
    boinc_resolve_filename_s("output.txt", resolved_output, sizeof(resolved_output));
    input_file = resolved_input;
    output_file = resolved_output;
#else
    // Command line arguments for standalone testing
    if (argc > 1) input_file = argv[1];
    if (argc > 2) output_file = argv[2];
#endif

    printf("Ulam-von Neumann Monte Carlo Solver for Ax = b\n");
    printf("==============================================\n\n");

    // Initialize random number generator
    init_random();

    // Read input
    printf("Reading input from %s...\n", input_file);
    if (read_input(input_file, &data) < 0) {
        fprintf(stderr, "Failed to read input\n");
#ifdef _BOINC_
        boinc_finish(1);
#endif
        return 1;
    }

    printf("System dimension: %d x %d\n", data.n, data.n);
    printf("Computing components: %d to %d\n", data.start_idx, data.end_idx);
    printf("Number of walks per component: %ld\n\n", data.num_walks);

    // Prepare iteration form
    if (prepare_iteration_form(&data) < 0) {
        fprintf(stderr, "Failed to prepare iteration form\n");
#ifdef _BOINC_
        boinc_finish(1);
#endif
        return 1;
    }

    // Compute solution
    compute_solution(&data, x_partial);

    // Verify if complete solution
    if (data.start_idx == 0 && data.end_idx == data.n - 1) {
        verify_solution(&data, x_partial);
    }

    // Write output
    printf("\nWriting output to %s...\n", output_file);
    if (write_output(output_file, &data, x_partial) < 0) {
        fprintf(stderr, "Failed to write output\n");
#ifdef _BOINC_
        boinc_finish(1);
#endif
        return 1;
    }

    printf("Done!\n");

#ifdef _BOINC_
    boinc_finish(0);
#endif

    return 0;
}
