# BOINC Administration Tools

This directory contains tools for managing the BOINC server.

## Files

### generate_work.py
Python script to generate multiple work units automatically.

**Purpose:**
Instead of manually creating work units one at a time with `bin/create_work`, this script automates the process for batch work generation.

**Requirements:**
- Python 3.6+
- BOINC project already set up
- Templates in `~/boinc_test/projects/templates/`

**Usage:**

```bash
# Generate 100 work units with 100 million iterations each
./generate_work.py --num_wu 100 --iterations 100000000

# Generate 50 work units with varying iterations from 10M to 1B
./generate_work.py --range 10000000:1000000000:50

# Use custom project directory
./generate_work.py --project_dir ~/boinc_test/projects --num_wu 10 --iterations 50000000

# Get help
./generate_work.py --help
```

**Features:**

1. **Fixed iteration count:**
   ```bash
   ./generate_work.py --num_wu 100 --iterations 10000000
   ```
   Creates 100 work units, each with 10M iterations

2. **Variable iteration count:**
   ```bash
   ./generate_work.py --range 1000000:100000000:20
   ```
   Creates 20 work units with iterations evenly distributed from 1M to 100M

3. **Automatic resource estimation:**
   - Calculates FPOPS based on iteration count
   - Sets appropriate memory and disk bounds
   - Configures 24-hour deadline

4. **Error handling:**
   - Validates project directory exists
   - Reports failed work unit creation
   - Summary statistics at end

**How it works:**

```python
# For each work unit:
1. Create input file with iteration count
2. Generate unique work unit name
3. Call bin/create_work with proper parameters
4. Report success/failure
```

**Example output:**

```
Generating 10 work units with 50000000 iterations each...
======================================================================
Created input file: pi_in_pi_wu_50000000_000000.txt (50000000 iterations)
✓ Created work unit: pi_wu_50000000_000000
Created input file: pi_in_pi_wu_50000000_000001.txt (50000000 iterations)
✓ Created work unit: pi_wu_50000000_000001
...
======================================================================
Summary: 10/10 work units created successfully

✓ Work generation complete!

Next steps:
  1. Check work units: cd ~/boinc_test/projects && bin/db_query 'SELECT COUNT(*) FROM workunit'
  2. Ensure daemons are running: bin/status
  3. Monitor progress: tail -f log_*/scheduler.log
```

**Customization:**

Edit the script to change:
- Default quorum (min_quorum = 2)
- Target results (target_nresults = 2)
- Resource bounds (memory, disk, FPOPS)
- Delay between creation (time.sleep)

**Troubleshooting:**

**Error: "Project directory not found"**
- Verify path: `ls ~/boinc_test/projects`
- Use --project_dir to specify correct path

**Error: "create_work failed"**
- Check templates exist: `ls ~/boinc_test/projects/templates/`
- Verify bin/create_work is executable
- Check MySQL database is running

**Work units created but not sent to clients:**
- Check daemons: `~/boinc_test/projects/bin/status`
- Check feeder: `tail -f ~/boinc_test/projects/log_*/feeder.log`
- Restart daemons if needed

## Advanced Usage

### Batch Processing

Create work for different configurations:

```bash
# Create work for accuracy testing
./generate_work.py --range 1000000:1000000000:10

# Create quick test work
./generate_work.py --num_wu 50 --iterations 1000000

# Create long-running work
./generate_work.py --num_wu 10 --iterations 10000000000
```

### Integration with Cron

Automatically generate work daily:

```bash
# Add to crontab
0 0 * * * cd /path/to/tools && ./generate_work.py --num_wu 100 --iterations 100000000 >> /tmp/work_gen.log 2>&1
```

### Custom Work Generation

Modify the script for specific needs:

```python
# Example: Generate work with specific naming
def generate_custom_work(self, prefix):
    for i in range(100):
        wu_name = f"{prefix}_wu_{i}"
        # ... create work
```

## Other Useful Commands

### Check Work Status

```bash
# Count work units
cd ~/boinc_test/projects
bin/db_query "SELECT COUNT(*) FROM workunit"

# Count unsent results
bin/db_query "SELECT COUNT(*) FROM result WHERE server_state=2"

# List recent work units
bin/db_query "SELECT id, name, create_time FROM workunit ORDER BY create_time DESC LIMIT 10"
```

### Monitor Server

```bash
# Check daemon status
~/boinc_test/projects/bin/status

# Watch scheduler activity
tail -f ~/boinc_test/projects/log_*/scheduler.log

# Watch work distribution
tail -f ~/boinc_test/projects/log_*/feeder.log
```

### Clean Up Old Work

```bash
# Delete old work units (CAREFUL!)
cd ~/boinc_test/projects
bin/db_query "DELETE FROM result WHERE server_state=5 AND create_time < UNIX_TIMESTAMP(NOW() - INTERVAL 7 DAY)"
bin/db_query "DELETE FROM workunit WHERE canonical_resultid > 0 AND create_time < UNIX_TIMESTAMP(NOW() - INTERVAL 7 DAY)"
```

## Learning Exercises

### Easy
- Generate 10 work units with 1M iterations each
- Create work with varying iterations (1M to 10M)
- Check how many work units are in the database

### Medium
- Modify script to generate work with specific naming pattern
- Add command-line option for custom quorum
- Create wrapper script for different iteration presets

### Advanced
- Integrate with web interface for work submission
- Add work priority levels
- Implement work scheduling based on client availability
- Create monitoring dashboard

## References

- BOINC Work Creation: https://github.com/BOINC/boinc/wiki/CreateWork
- Python subprocess: https://docs.python.org/3/library/subprocess.html
- argparse module: https://docs.python.org/3/library/argparse.html
