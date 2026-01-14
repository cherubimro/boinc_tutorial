/*
 * BOINC PI Computation Application
 *
 * This is an educational example application for BOINC that computes
 * the value of PI using the Monte Carlo method.
 *
 * The Monte Carlo method works by randomly sampling points in a square
 * and determining how many fall within a quarter circle inscribed in it.
 * The ratio of points inside the circle to total points approximates PI/4.
 *
 * This example demonstrates:
 * - BOINC API initialization and finalization
 * - Reading input from a file
 * - Writing output to a file
 * - Progress reporting
 * - Checkpointing for fault tolerance
 * - Fraction done updates
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#include "boinc_api.h"
#include "filesys.h"
#include "util.h"

// Structure to hold our checkpoint data
struct CHECKPOINT_DATA {
    long long iterations_completed;
    long long points_in_circle;
    unsigned int random_seed;
};

// Global variables
CHECKPOINT_DATA checkpoint_data;
long long total_iterations = 0;

// Function to read input file
int read_input_file(const char* filename, long long& iterations) {
    FILE* infile;
    int retval;
    char input_path[512];

    // Resolve the logical filename to physical path
    retval = boinc_resolve_filename(filename, input_path, sizeof(input_path));
    if (retval) {
        fprintf(stderr, "APP: error resolving input filename %s\n", filename);
        return retval;
    }

    infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr, "APP: error opening input file %s\n", input_path);
        return -1;
    }

    // Read the number of iterations from the input file
    if (fscanf(infile, "%lld", &iterations) != 1) {
        fprintf(stderr, "APP: error reading iterations from input file\n");
        fclose(infile);
        return -1;
    }

    fclose(infile);
    fprintf(stderr, "APP: input file read successfully. Iterations: %lld\n", iterations);
    return 0;
}

// Function to write output file
int write_output_file(const char* filename, double pi_estimate, long long iterations) {
    FILE* outfile;
    int retval;
    char output_path[512];

    // Resolve the logical filename to physical path
    retval = boinc_resolve_filename(filename, output_path, sizeof(output_path));
    if (retval) {
        fprintf(stderr, "APP: error resolving output filename %s\n", filename);
        return retval;
    }

    outfile = boinc_fopen(output_path, "w");
    if (!outfile) {
        fprintf(stderr, "APP: error opening output file %s\n", output_path);
        return -1;
    }

    // Write results
    fprintf(outfile, "PI Computation Results\n");
    fprintf(outfile, "======================\n");
    fprintf(outfile, "Total iterations: %lld\n", iterations);
    fprintf(outfile, "Estimated value of PI: %.15f\n", pi_estimate);
    fprintf(outfile, "Error from actual PI: %.15f\n", fabs(pi_estimate - M_PI));
    fprintf(outfile, "Accuracy: %.10f%%\n", 100.0 * (1.0 - fabs(pi_estimate - M_PI) / M_PI));

    fclose(outfile);
    fprintf(stderr, "APP: output file written successfully\n");
    return 0;
}

// Function to write checkpoint file
int write_checkpoint(const char* filename, const CHECKPOINT_DATA& data) {
    FILE* checkpoint_file;
    int retval;
    char checkpoint_path[512];

    retval = boinc_resolve_filename(filename, checkpoint_path, sizeof(checkpoint_path));
    if (retval) {
        fprintf(stderr, "APP: error resolving checkpoint filename\n");
        return retval;
    }

    checkpoint_file = boinc_fopen(checkpoint_path, "w");
    if (!checkpoint_file) {
        fprintf(stderr, "APP: error opening checkpoint file for writing\n");
        return -1;
    }

    fprintf(checkpoint_file, "%lld %lld %u\n",
            data.iterations_completed,
            data.points_in_circle,
            data.random_seed);

    fclose(checkpoint_file);
    return 0;
}

// Function to read checkpoint file
int read_checkpoint(const char* filename, CHECKPOINT_DATA& data) {
    FILE* checkpoint_file;
    int retval;
    char checkpoint_path[512];

    retval = boinc_resolve_filename(filename, checkpoint_path, sizeof(checkpoint_path));
    if (retval) {
        // Checkpoint file doesn't exist, start from beginning
        return -1;
    }

    checkpoint_file = boinc_fopen(checkpoint_path, "r");
    if (!checkpoint_file) {
        // Checkpoint file doesn't exist, start from beginning
        return -1;
    }

    if (fscanf(checkpoint_file, "%lld %lld %u",
               &data.iterations_completed,
               &data.points_in_circle,
               &data.random_seed) != 3) {
        fprintf(stderr, "APP: error reading checkpoint file\n");
        fclose(checkpoint_file);
        return -1;
    }

    fclose(checkpoint_file);
    fprintf(stderr, "APP: checkpoint read successfully. Resuming from iteration %lld\n",
            data.iterations_completed);
    return 0;
}

// Main computation function - Monte Carlo PI estimation
int compute_pi() {
    int retval;

    // Read input file to get number of iterations
    retval = read_input_file("in", total_iterations);
    if (retval) {
        return retval;
    }

    // Try to read checkpoint file
    if (read_checkpoint("checkpoint.txt", checkpoint_data) != 0) {
        // No checkpoint found, initialize from scratch
        checkpoint_data.iterations_completed = 0;
        checkpoint_data.points_in_circle = 0;

        // Use /dev/urandom for truly random seed
        FILE* urandom = fopen("/dev/urandom", "rb");
        if (urandom) {
            if (fread(&checkpoint_data.random_seed, sizeof(checkpoint_data.random_seed), 1, urandom) != 1) {
                fprintf(stderr, "APP: warning - failed to read from /dev/urandom, using fallback\n");
                // Fallback to time + PID if /dev/urandom fails
                struct timeval tv;
                gettimeofday(&tv, NULL);
                checkpoint_data.random_seed = (unsigned int)(tv.tv_sec * 1000000 + tv.tv_usec + getpid());
            }
            fclose(urandom);
        } else {
            fprintf(stderr, "APP: warning - cannot open /dev/urandom, using fallback\n");
            // Fallback to time + PID if /dev/urandom cannot be opened
            struct timeval tv;
            gettimeofday(&tv, NULL);
            checkpoint_data.random_seed = (unsigned int)(tv.tv_sec * 1000000 + tv.tv_usec + getpid());
        }

        fprintf(stderr, "APP: starting computation from beginning with seed %u\n",
                checkpoint_data.random_seed);
    } else {
        fprintf(stderr, "APP: resuming from checkpoint\n");
    }

    // Initialize random number generator
    srand(checkpoint_data.random_seed);

    // Main computation loop
    for (long long i = checkpoint_data.iterations_completed; i < total_iterations; i++) {
        // Generate random point in unit square
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;

        // Check if point is inside quarter circle
        if (x * x + y * y <= 1.0) {
            checkpoint_data.points_in_circle++;
        }

        checkpoint_data.iterations_completed = i + 1;

        // Update progress and check for BOINC events every 100,000 iterations
        if (i % 100000 == 0) {
            double fraction_done = (double)i / total_iterations;
            boinc_fraction_done(fraction_done);

            // Allow BOINC to suspend/resume the application
            boinc_sleep(0);
        }

        // Check if it's time to checkpoint
        if (boinc_time_to_checkpoint()) {
            // Update random seed for next resume
            checkpoint_data.random_seed = rand();

            retval = write_checkpoint("checkpoint.txt", checkpoint_data);
            if (retval) {
                fprintf(stderr, "APP: checkpoint write failed\n");
                return retval;
            }

            boinc_checkpoint_completed();
            fprintf(stderr, "APP: checkpoint written at iteration %lld\n", i);
        }
    }

    // Computation complete, calculate PI
    double pi_estimate = 4.0 * checkpoint_data.points_in_circle / total_iterations;

    fprintf(stderr, "APP: computation complete\n");
    fprintf(stderr, "APP: Points in circle: %lld\n", checkpoint_data.points_in_circle);
    fprintf(stderr, "APP: Total points: %lld\n", total_iterations);
    fprintf(stderr, "APP: Estimated PI: %.15f\n", pi_estimate);

    // Write output file
    retval = write_output_file("out", pi_estimate, total_iterations);
    if (retval) {
        return retval;
    }

    // Report 100% completion
    boinc_fraction_done(1.0);

    return 0;
}

int main(int argc, char** argv) {
    int retval;

    // Initialize BOINC
    // Using simple boinc_init() for standalone mode compatibility
    retval = boinc_init();
    if (retval) {
        fprintf(stderr, "APP: boinc_init() failed: %d\n", retval);
        fprintf(stderr, "APP: This may be normal for standalone testing\n");
        // Continue anyway - most BOINC functions will still work
    }

    fprintf(stderr, "APP: PI Computation started\n");

    // Run the main computation
    retval = compute_pi();

    if (retval) {
        fprintf(stderr, "APP: computation failed with error %d\n", retval);
        boinc_finish(retval);
    } else {
        fprintf(stderr, "APP: computation completed successfully\n");
        boinc_finish(0);
    }

    return 0;
}
