/*
 * PI Computation Validator for BOINC
 *
 * This validator compares PI computation results from different volunteers.
 * Since the Monte Carlo method produces slightly different results each time
 * (due to randomness), we need a tolerant comparison rather than exact match.
 *
 * Compile:
 *   g++ -o pi_validator pi_validator.cpp -I/path/to/boinc/sched -lboinc_sched -lboinc
 *
 * Deploy:
 *   Copy to ~/projects/pi_compute/bin/
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include "sched_msgs.h"
#include "validate_util.h"
#include "validate_util2.h"

using std::vector;

// Tolerance for PI comparison (0.1% relative error)
const double PI_TOLERANCE = 0.001;

/**
 * Parse PI value from output file
 *
 * Expected format:
 *   PI Computation Results
 *   ======================
 *   Total iterations: NNNNNN
 *   Estimated value of PI: X.XXXXXXXXXXXXXXX
 *   ...
 */
double parse_pi_from_output(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        log_messages.printf(MSG_CRITICAL,
            "[pi_validator] Cannot open output file: %s\n", path);
        return -1.0;
    }

    char line[256];
    double pi_value = -1.0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Estimated value of PI:")) {
            // Parse the PI value
            char* colon = strchr(line, ':');
            if (colon) {
                pi_value = atof(colon + 1);
                break;
            }
        }
    }

    fclose(f);

    if (pi_value < 0) {
        log_messages.printf(MSG_CRITICAL,
            "[pi_validator] Could not parse PI value from: %s\n", path);
    } else {
        log_messages.printf(MSG_DEBUG,
            "[pi_validator] Parsed PI value: %.15f from %s\n", pi_value, path);
    }

    return pi_value;
}

/**
 * Initialize validation
 * Called once at startup
 */
int init_result(RESULT& result, void*& data) {
    int retval;

    // Get the output file
    vector<FILE_INFO> fis;
    retval = get_output_file_infos(result, fis);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[pi_validator] get_output_file_infos() failed: %d\n", retval);
        return retval;
    }

    if (fis.size() != 1) {
        log_messages.printf(MSG_CRITICAL,
            "[pi_validator] Expected 1 output file, got %d\n", (int)fis.size());
        return ERR_XML_PARSE;
    }

    // Get file path
    const char* path = fis[0].path.c_str();

    // Parse PI value
    double* pi_ptr = new double;
    *pi_ptr = parse_pi_from_output(path);

    if (*pi_ptr < 0) {
        delete pi_ptr;
        return ERR_XML_PARSE;
    }

    data = (void*)pi_ptr;
    return 0;
}

/**
 * Compare two results
 *
 * Returns:
 *   0: Results match (within tolerance)
 *   1: Results don't match
 */
int compare_results(
    RESULT& r1, void* data1,
    RESULT& r2, void* data2,
    bool& match
) {
    double pi1 = *(double*)data1;
    double pi2 = *(double*)data2;

    // Calculate relative difference
    double diff = fabs(pi1 - pi2);
    double avg = (pi1 + pi2) / 2.0;
    double relative_error = diff / avg;

    log_messages.printf(MSG_NORMAL,
        "[pi_validator] Comparing results:\n");
    log_messages.printf(MSG_NORMAL,
        "  Result 1 (%s): PI = %.15f\n", r1.name, pi1);
    log_messages.printf(MSG_NORMAL,
        "  Result 2 (%s): PI = %.15f\n", r2.name, pi2);
    log_messages.printf(MSG_NORMAL,
        "  Difference: %.15f (%.6f%%)\n", diff, relative_error * 100.0);

    // Check if within tolerance
    if (relative_error <= PI_TOLERANCE) {
        match = true;
        log_messages.printf(MSG_NORMAL,
            "[pi_validator] Results MATCH (within %.2f%% tolerance)\n",
            PI_TOLERANCE * 100.0);
    } else {
        match = false;
        log_messages.printf(MSG_NORMAL,
            "[pi_validator] Results DO NOT MATCH (exceeds %.2f%% tolerance)\n",
            PI_TOLERANCE * 100.0);
    }

    return 0;
}

/**
 * Cleanup validation data
 */
int cleanup_result(RESULT const& /*result*/, void* data) {
    if (data) {
        delete (double*)data;
    }
    return 0;
}

/**
 * Check if a result set is valid
 * (all results agree within tolerance)
 */
bool check_set(
    vector<RESULT>& /*results*/,
    WORKUNIT& /*wu*/,
    int& /*canonicalid*/,
    double& /*credit*/,
    bool& /*retry*/
) {
    // Use default behavior
    retry = false;
    return false;
}

/**
 * Check if result needs more processing
 */
bool check_pair(
    RESULT& /*r1*/,
    RESULT& /*r2*/,
    bool& /*retry*/
) {
    retry = false;
    return false;
}

/**
 * Main validation driver
 */
const char* BOINC_RCSID_33c7876 = "$Id$";

int main(int argc, char** argv) {
    log_messages.set_debug_level(3);
    log_messages.printf(MSG_NORMAL, "PI Validator starting...\n");

    return validate_handler(argc, argv);
}
