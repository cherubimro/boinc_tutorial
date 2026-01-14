# BOINC Server Components

This directory contains advanced server components for BOINC project administration.

## Files

### pi_validator.cpp
Custom validator for PI computation results.

**Purpose:**
BOINC sends the same work unit to multiple clients for cross-validation. The validator compares results from different clients to detect errors or cheating.

**How it works:**
- Parses PI values from result files
- Compares results from different clients
- Allows 0.1% relative error (due to Monte Carlo randomness)
- Marks results as valid if they agree within tolerance

**Building:**
```bash
# Requires BOINC development headers
g++ -o pi_validator pi_validator.cpp \
    -I~/boinc_source/sched \
    -L~/boinc_source/sched/.libs \
    -L~/boinc_source/lib/.libs \
    -lboinc_sched -lboinc -pthread
```

**Deployment:**
```bash
cp pi_validator ~/boinc_test/projects/bin/
cd ~/boinc_test/projects
# Configure project.xml to use custom validator
# Restart daemons: bin/stop && bin/start
```

**Why this is needed:**
The default validator does exact binary comparison, which fails for Monte Carlo results because:
- Each client uses different random seeds (/dev/urandom)
- Results are intentionally different
- We need tolerance-based comparison instead

**Configuration in project.xml:**
```xml
<daemon>
    <cmd>pi_validator --app pi_compute</cmd>
    <output>validator_pi.log</output>
</daemon>
```

**Testing:**
```bash
# Check validator logs
tail -f ~/boinc_test/projects/log_*/validator_pi.log

# Expected output:
# [pi_validator] Comparing results:
#   Result 1: PI = 3.141630400000000
#   Result 2: PI = 3.141109600000000
#   Difference: 0.000520800000000 (0.016576%)
# [pi_validator] Results MATCH (within 0.10% tolerance)
```

## Advanced Topics

### Custom Validators

BOINC supports several validation strategies:

1. **Exact match** (default) - Binary comparison
2. **Bitwise comparison** - Exact numerical match
3. **Tolerance-based** (pi_validator) - Allow small differences
4. **Majority voting** - Accept result agreed by majority
5. **Trusted users** - Weight results by user reputation

### When to Use Custom Validators

- **Scientific computing:** Floating-point rounding differences
- **Randomized algorithms:** Different seeds produce different results
- **Approximate algorithms:** Results within acceptable error margin
- **Graphics rendering:** Pixel-level differences acceptable

### Validator Development

Key functions to implement:

```cpp
// Initialize result data structure
int init_result(RESULT& result, void*& data);

// Compare two results
int compare_results(
    RESULT& r1, void* data1,
    RESULT& r2, void* data2,
    bool& match
);

// Cleanup
int cleanup_result(RESULT const& result, void* data);
```

### Integration with BOINC

The validator runs as a daemon:
- Periodically scans for work units needing validation
- Compares all results for each work unit
- Marks canonical (correct) result
- Grants credit to users with matching results
- Marks bad results as invalid

### Validation States

Results go through these states:
1. **UNSENT** - Not yet sent to client
2. **IN_PROGRESS** - Client computing
3. **OVER, need_validate** - Waiting for validation
4. **OVER, validated** - Validator accepted
5. **OVER, invalid** - Validator rejected

### Troubleshooting

**All results marked invalid:**
- Check validator tolerance (PI_TOLERANCE = 0.001)
- Verify parsing logic (parse_pi_from_output)
- Check output format matches expected

**Validator not running:**
```bash
# Check daemon status
~/boinc_test/projects/bin/status

# Check logs
tail -f ~/boinc_test/projects/log_*/validator_pi.log

# Restart daemons
~/boinc_test/projects/bin/stop
~/boinc_test/projects/bin/start
```

**No results being validated:**
- Need minimum quorum (usually 2-3 results)
- Check `target_nresults` in work unit creation
- Verify multiple clients are processing

## Learning Exercises

### Easy
- Modify tolerance from 0.1% to 0.5%
- Add more detailed logging
- Count how many validations pass vs. fail

### Medium
- Implement majority voting (3+ results)
- Add result caching to speed up validation
- Weight results by user reliability

### Advanced
- Implement adaptive tolerance based on iteration count
- Add statistical analysis of result distribution
- Build validator for different application (e.g., e computation)

## References

- BOINC Validation: https://github.com/BOINC/boinc/wiki/ValidationDesign
- Validator API: See `sched/validate_util.h` in BOINC source
- Example validators: `sched/sample_*_validator.cpp` in BOINC source
