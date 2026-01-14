/*
 * axb_validator.cpp
 *
 * BOINC validator for Ax=b Monte Carlo solver
 * Merges partial solutions from different work units into complete solution
 *
 * Each work unit computes a subset of solution components.
 * The validator collects all components and verifies the complete solution.
 *
 * Licensed under GPL v3
 */

#include <vector>
#include <map>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "sched_config.h"
#include "sched_util.h"
#include "validate_util.h"
#include "validator.h"

using std::vector;
using std::map;

// Structure to hold partial solution from one work unit
struct PartialSolution {
    int start_idx;
    int end_idx;
    vector<double> values;
};

// Parse output file to extract partial solution
int parse_result(const char* path, PartialSolution& sol) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        log_messages.printf(MSG_CRITICAL, "Cannot open result file %s\n", path);
        return -1;
    }

    // Read component range
    if (fscanf(fp, "%d %d", &sol.start_idx, &sol.end_idx) != 2) {
        log_messages.printf(MSG_CRITICAL, "Cannot read component range from %s\n", path);
        fclose(fp);
        return -1;
    }

    // Read values
    int num_components = sol.end_idx - sol.start_idx + 1;
    sol.values.resize(num_components);

    for (int i = 0; i < num_components; i++) {
        if (fscanf(fp, "%lf", &sol.values[i]) != 1) {
            log_messages.printf(MSG_CRITICAL,
                "Cannot read value %d from %s\n", i, path);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

// Compare two partial solutions for the same component range
// Returns true if they agree within tolerance
bool compare_partial_solutions(const PartialSolution& sol1,
                               const PartialSolution& sol2,
                               double tolerance = 0.01) {

    if (sol1.start_idx != sol2.start_idx || sol1.end_idx != sol2.end_idx) {
        return false;  // Different ranges
    }

    if (sol1.values.size() != sol2.values.size()) {
        return false;
    }

    // Compare values component by component
    for (size_t i = 0; i < sol1.values.size(); i++) {
        double diff = fabs(sol1.values[i] - sol2.values[i]);
        double avg = (fabs(sol1.values[i]) + fabs(sol2.values[i])) / 2.0;

        // Use relative error if values are not too small
        double error = (avg > 1e-10) ? (diff / avg) : diff;

        if (error > tolerance) {
            log_messages.printf(MSG_NORMAL,
                "Component %d differs: %.10e vs %.10e (error: %.10e)\n",
                sol1.start_idx + (int)i, sol1.values[i], sol2.values[i], error);
            return false;
        }
    }

    return true;
}

// Check if a set of results form a complete, valid solution
int check_set(
    vector<RESULT>& results,
    WORKUNIT& wu,
    int& canonicalid,
    double& credit,
    bool& retry
) {
    retry = false;

    // Parse all results
    vector<PartialSolution> solutions;
    map<int, int> component_coverage;  // Maps component index to solution index

    for (size_t i = 0; i < results.size(); i++) {
        PartialSolution sol;

        // Get output file path
        vector<OUTPUT_FILE_INFO> files;
        int retval = get_output_file_infos(results[i], files);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "Cannot get output files for result %lu\n", results[i].id);
            continue;
        }

        if (files.empty()) {
            log_messages.printf(MSG_CRITICAL,
                "No output files for result %lu\n", results[i].id);
            continue;
        }

        // Parse the output file
        if (parse_result(files[0].path.c_str(), sol) < 0) {
            log_messages.printf(MSG_CRITICAL,
                "Cannot parse result %lu\n", results[i].id);
            continue;
        }

        // Check for overlap with existing solutions
        bool valid = true;
        for (int idx = sol.start_idx; idx <= sol.end_idx; idx++) {
            if (component_coverage.find(idx) != component_coverage.end()) {
                // Component already covered - verify consistency
                int prev_sol_idx = component_coverage[idx];
                int local_idx = idx - sol.start_idx;
                int prev_local_idx = idx - solutions[prev_sol_idx].start_idx;

                double diff = fabs(sol.values[local_idx] -
                                  solutions[prev_sol_idx].values[prev_local_idx]);
                double avg = (fabs(sol.values[local_idx]) +
                             fabs(solutions[prev_sol_idx].values[prev_local_idx])) / 2.0;
                double error = (avg > 1e-10) ? (diff / avg) : diff;

                if (error > 0.01) {  // 1% tolerance
                    log_messages.printf(MSG_NORMAL,
                        "Inconsistent values for component %d: %.10e vs %.10e\n",
                        idx, sol.values[local_idx],
                        solutions[prev_sol_idx].values[prev_local_idx]);
                    valid = false;
                    break;
                }
            }
        }

        if (!valid) {
            continue;  // Skip this result due to inconsistency
        }

        // Add solution and mark components as covered
        int sol_idx = solutions.size();
        solutions.push_back(sol);

        for (int idx = sol.start_idx; idx <= sol.end_idx; idx++) {
            component_coverage[idx] = sol_idx;
        }
    }

    if (solutions.empty()) {
        log_messages.printf(MSG_CRITICAL, "No valid results in set\n");
        retry = true;
        return -1;
    }

    // Check if we have complete coverage
    // Determine the full range needed (assume components 0 to max_idx)
    int max_idx = 0;
    for (const auto& sol : solutions) {
        if (sol.end_idx > max_idx) max_idx = sol.end_idx;
    }

    bool complete = true;
    for (int idx = 0; idx <= max_idx; idx++) {
        if (component_coverage.find(idx) == component_coverage.end()) {
            log_messages.printf(MSG_NORMAL,
                "Component %d not covered by any result\n", idx);
            complete = false;
        }
    }

    if (!complete) {
        log_messages.printf(MSG_NORMAL,
            "Incomplete solution - not all components computed\n");
        retry = true;
        return -1;
    }

    // Success! Mark first result as canonical
    canonicalid = results[0].id;

    // Grant credit based on computational effort
    // More components or more work gets more credit
    credit = 0;
    for (const auto& sol : solutions) {
        int num_components = sol.end_idx - sol.start_idx + 1;
        credit += num_components * 10.0;  // 10 credits per component
    }

    log_messages.printf(MSG_NORMAL,
        "Valid complete solution with %zu partial results, granting %.2f credits\n",
        solutions.size(), credit);

    return 0;
}

// Required BOINC validator function
int init_result(RESULT& result, void*& data) {
    return 0;
}

// Required BOINC validator function
int compare_results(RESULT& r1, void* data1, RESULT& r2, void* data2, bool& match) {
    PartialSolution sol1, sol2;

    // Get output files
    vector<OUTPUT_FILE_INFO> files1, files2;
    get_output_file_infos(r1, files1);
    get_output_file_infos(r2, files2);

    if (files1.empty() || files2.empty()) {
        match = false;
        return 0;
    }

    // Parse results
    if (parse_result(files1[0].path.c_str(), sol1) < 0 ||
        parse_result(files2[0].path.c_str(), sol2) < 0) {
        match = false;
        return 0;
    }

    // Compare
    match = compare_partial_solutions(sol1, sol2);

    return 0;
}

// Required BOINC validator function
int cleanup_result(RESULT const& result, void* data) {
    return 0;
}

// Main validator entry point
int main(int argc, char** argv) {
    int retval;

    retval = sched_config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Cannot parse config file: %d\n", retval);
        return retval;
    }

    retval = boinc_db.open(
        sched_config.db_name,
        sched_config.db_host,
        sched_config.db_user,
        sched_config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Cannot open database: %d\n", retval);
        return retval;
    }

    log_messages.printf(MSG_NORMAL, "axb_validator: starting\n");

    retval = process_wu_results(argc, argv, check_set);

    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "process_wu_results returned %d\n", retval);
    }

    return retval;
}
