# BOINC PI Computation Tutorial

## Educational Distributed Computing Project

This tutorial demonstrates how to build a complete BOINC (Berkeley Open Infrastructure for Network Computing) distributed computing system that calculates the value of PI using the Monte Carlo method.

**Author:** **Author:** Alin-Adrian Anton <alin.anton@upt.ro> - Educational example for students
**Date:** January 2026
**BOINC Version:** 8.3.0

---

## Table of Contents

1. [Overview](#overview)
2. [System Requirements](#system-requirements)
3. [Quick Start](#quick-start)
4. [Detailed Installation](#detailed-installation)
5. [Understanding the Code](#understanding-the-code)
6. [Testing the System](#testing-the-system)
7. [Troubleshooting](#troubleshooting)
8. [Learning Objectives](#learning-objectives)
9. [Further Resources](#further-resources)

---

## Overview

### What is BOINC?

BOINC is an open-source middleware system for volunteer and grid computing. It allows you to create distributed computing projects where volunteer computers around the world can contribute processing power to solve complex problems.

### What Does This Project Do?

This project:
- **Computes PI** using the Monte Carlo method (random sampling)
- **Distributes work** across multiple client computers
- **Validates results** through cross-validation (multiple clients compute the same task)
- **Demonstrates fault tolerance** with checkpointing (resume after interruption)

### Architecture

```
┌──────────────────────────────────────────────────────────┐
│                    BOINC Server                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                │
│  │ Database │  │  Feeder  │  │Scheduler │                │
│  │ (MySQL)  │  │  Daemon  │  │  (CGI)   │                │
│  └──────────┘  └──────────┘  └──────────┘                │
│       │              │              │                    │
└───────┼──────────────┼──────────────┼────────────────────┘
        │              │              │
        └──────────────┴──────────────┴─────┐
                                            │
                  ┌─────────────────────────┼─────────────┐
                  │                         │             │
           ┌──────▼──────┐         ┌────────▼─────┐  ┌────▼────┐
           │  Client A   │         │   Client B   │  │Client C │
           │ (Computer 1)│         │ (Computer 2) │  │(Computer│
           └─────────────┘         └──────────────┘  └─────────┘
                  │                         │             │
                  └─────────────────────────┴─────────────┘
                          Compute PI Results
```

---

## System Requirements

### Minimum Hardware
- **CPU:** 2+ cores (8 cores recommended for testing multiple clients)
- **RAM:** 4 GB minimum (8 GB recommended)
- **Disk:** 10 GB free space
- **Network:** Localhost sufficient for testing

### Operating System
- **Linux:** Ubuntu 20.04+, Debian 11+, openSUSE Leap 15.4+, Fedora 35+
- **Windows:** WSL2 (Windows Subsystem for Linux) with Ubuntu 20.04+
- **macOS:** 10.15+ (with Homebrew)

### Required Software
All dependencies are documented in the installation scripts provided.

---

## Quick Start

For experienced users who want to get started immediately:

```bash
# Extract the archive
tar -xzf boinc_pi_tutorial.tar.gz
cd boinc_pi_tutorial

# Run the automated setup (Ubuntu/Debian)
./scripts/install_dependencies_debian.sh
./scripts/build_boinc.sh
./scripts/setup_server.sh
./scripts/start_clients.sh

# Check results
./scripts/check_results.sh
```

For detailed step-by-step instructions, continue reading below.

---

## Detailed Installation

### Step 1: Install System Dependencies

Choose the script for your Linux distribution:

#### Ubuntu/Debian
```bash
cd boinc_pi_tutorial
sudo ./scripts/install_dependencies_debian.sh
```

#### openSUSE/SUSE
```bash
cd boinc_pi_tutorial
sudo ./scripts/install_dependencies_opensuse.sh
```

#### Fedora/RHEL
```bash
cd boinc_pi_tutorial
sudo ./scripts/install_dependencies_fedora.sh
```

#### Windows WSL2
```bash
# First install WSL2 with Ubuntu 20.04+
# Then use the Ubuntu/Debian script
sudo ./scripts/install_dependencies_debian.sh
```

**What this installs:**
- Build tools (g++, make, autotools)
- BOINC development libraries
- MariaDB/MySQL database
- Apache web server
- PHP with MySQL support
- Python with database libraries
- JSON and networking libraries

### Step 2: Build BOINC from Source

```bash
./scripts/build_boinc.sh
```

This script:
1. Clones the BOINC repository from GitHub
2. Configures BOINC with server support
3. Compiles the server and client components
4. Takes approximately 10-15 minutes on modern hardware

**Note:** The build uses `-j1` (single-threaded) to avoid race conditions. This is slower but more reliable.

### Step 3: Build the PI Computation Application

```bash
cd src
make
```

This compiles `pi_compute.cpp` which:
- Uses BOINC API for distributed computing
- Implements Monte Carlo PI estimation
- Includes checkpointing for fault tolerance
- Uses `/dev/urandom` for truly random seeds

### Step 4: Set Up the BOINC Server

```bash
./scripts/setup_server.sh
```

This script:
1. Creates MySQL database and user
2. Creates BOINC project structure
3. Configures Apache web server
4. Deploys the PI computation application
5. Starts server daemons (feeder, transitioner, file_deleter)
6. Creates initial work units

**Interactive prompts:**
- MySQL root password (set during dependency installation)
- Project name (default: `pi_test`)
- Database password (default: `boinc123`)

### Step 5: Start BOINC Clients

```bash
./scripts/start_clients.sh
```

This script creates and starts 3 BOINC clients on different ports:
- **Client A:** localhost:31421
- **Client B:** localhost:31422
- **Client C:** localhost:31423

Each client:
- Runs independently
- Connects to the local server
- Downloads and executes work units
- Reports results back to the server

---

## Understanding the Code

### The Monte Carlo Method

The Monte Carlo method estimates PI by random sampling:

```
1. Generate random points in a 1x1 square
2. Count how many fall inside a quarter-circle (radius = 1)
3. Ratio = (points_in_circle / total_points) ≈ PI/4
4. Therefore: PI ≈ 4 × ratio
```

**Example with 10 million samples:**
```
Points in circle: 7,853,981
Total points: 10,000,000
Ratio: 0.7853981
PI ≈ 4 × 0.7853981 = 3.1415924
Actual PI: 3.1415926...
Accuracy: 99.999%
```

### Key BOINC API Functions

#### Initialization and Cleanup
```cpp
boinc_init();           // Initialize BOINC application
boinc_finish(status);   // Terminate with status code
```

#### Progress Reporting
```cpp
boinc_fraction_done(0.5);  // Report 50% complete
```

#### File Operations
```cpp
boinc_resolve_filename("in", path, sizeof(path));  // Map logical to physical filename
FILE* f = boinc_fopen(path, "r");                  // BOINC-aware file open
```

#### Checkpointing (Fault Tolerance)
```cpp
if (boinc_time_to_checkpoint()) {
    // Save current state
    write_checkpoint(data);
    boinc_checkpoint_completed();
}
```

#### Random Seed Generation
```cpp
// Using /dev/urandom for truly random seeds
FILE* urandom = fopen("/dev/urandom", "rb");
fread(&seed, sizeof(seed), 1, urandom);
fclose(urandom);
srand(seed);
```

### Code Structure

**pi_compute.cpp** (main application):
```
main()
├── boinc_init()               // Initialize BOINC
├── compute_pi()
│   ├── read_input_file()      // Read iteration count
│   ├── read_checkpoint()      // Resume if interrupted
│   ├── [Monte Carlo loop]     // Main computation
│   │   ├── Generate random point
│   │   ├── Check if in circle
│   │   ├── Update progress
│   │   └── Checkpoint if needed
│   └── write_output_file()    // Save PI result
└── boinc_finish()             // Clean termination
```

---

## Testing the System

### Check Server Status

```bash
./scripts/check_server_status.sh
```

This shows:
- Server daemon status (feeder, transitioner, file_deleter)
- Database connection
- Work units in database
- Apache configuration

### Check Client Status

```bash
./scripts/check_client_status.sh
```

This displays:
- Running clients and their PIDs
- Work units being processed
- Completed tasks
- Current computation progress

### View Results

```bash
./scripts/check_results.sh
```

This queries the database and shows:
- All completed work units
- PI values computed by each client
- Accuracy percentages
- Cross-validation status

**Example output:**
```
Work Unit: pi_wu_17
  Result 0: PI = 3.141630400 (99.9988% accuracy)
  Result 1: PI = 3.141109600 (99.9846% accuracy)
  Result 2: PI = 3.141212000 (99.9879% accuracy)
Status: All 3 clients produced different results (cross-validation working)
```

### Create New Work Units

```bash
./scripts/create_work.sh 20000000
```

Creates a new work unit with 20 million iterations. The script:
1. Creates input file with iteration count
2. Runs `create_work` with proper templates
3. Sets up 3 result instances for cross-validation

### Stop Everything

```bash
./scripts/stop_all.sh
```

Stops:
- All BOINC clients
- Server daemons
- Apache (optional)

---

## Troubleshooting

### Problem: Clients not getting work

**Symptoms:**
```
[pi_test] Scheduler request completed: got 0 new tasks
[pi_test] Project has no tasks available
```

**Solutions:**
1. Check server daemons are running:
   ```bash
   ps aux | grep -E "feeder|transitioner"
   ```

2. Create new work units:
   ```bash
   ./scripts/create_work.sh 10000000
   ```

3. Check database for available work:
   ```bash
   mysql -u boincadm -pboinc123 pi_test -e "SELECT name, server_state FROM result LIMIT 10"
   ```

### Problem: Permission denied errors

**Symptoms:**
```
Error reported by file upload server: can't write to upload_dir
```

**Solution:**
```bash
chmod 777 ~/boinc_test/projects/upload
chmod 777 ~/boinc_test/projects/download
```

### Problem: Platform not supported

**Symptoms:**
```
This project doesn't support computers of type x86_64-pc-linux-gnu
```

**Solution:**
```bash
cd ~/boinc_test/projects
bin/update_versions
```

### Problem: MySQL connection errors

**Symptoms:**
```
ERROR 1045 (28000): Access denied for user 'boincadm'@'localhost'
```

**Solution:**
```bash
mysql -u root -p << EOF
CREATE USER IF NOT EXISTS 'boincadm'@'localhost' IDENTIFIED BY 'boinc123';
GRANT ALL PRIVILEGES ON pi_test.* TO 'boincadm'@'localhost';
FLUSH PRIVILEGES;
EOF
```

### Problem: Clients crash immediately

**Symptoms:**
```
SIGSEGV: segmentation violation
```

**Solution:**
Use the custom-built client instead of system package:
```bash
~/boinc_source/client/boinc_client --dir ~/boinc_clients/client_a --allow_remote_gui_rpc
```

### Problem: All clients produce identical results

**Symptoms:**
```
All 3 clients: PI = 3.142452 (exactly the same)
```

**Cause:** Random seed not unique (fixed in this version)

**Verification:**
```bash
# Check that binary uses /dev/urandom
strings ~/boinc_test/projects/apps/pi_compute/1.0/x86_64-pc-linux-gnu/pi_compute_1.0_x86_64-pc-linux-gnu | grep urandom
```

### Debug Logs

Important log files:
```
Server logs:
  ~/boinc_test/projects/log_192/scheduler.log
  ~/boinc_test/projects/log_192/feeder.log
  ~/boinc_test/projects/log_192/transitioner.log

Client logs:
  ~/boinc_clients/client_a/client_a.log
  ~/boinc_clients/client_b/client_b.log
  ~/boinc_clients/client_c/client_c.log

Apache logs:
  /var/log/apache2/error.log (Debian/Ubuntu)
  /var/log/apache/error_log (openSUSE)
```

---

## Learning Objectives

After completing this tutorial, students will understand:

### 1. Distributed Computing Concepts
- **Task parallelism:** Breaking large problems into independent tasks
- **Work distribution:** Server assigns tasks to available clients
- **Result collection:** Aggregating results from multiple sources
- **Fault tolerance:** Handling client failures and network issues

### 2. BOINC Architecture
- **Client-server model:** Clear separation of roles
- **Database-driven:** MySQL stores all state
- **HTTP-based communication:** Standard web protocols
- **Daemon processes:** Background services for work management

### 3. Scientific Computing
- **Monte Carlo methods:** Random sampling for estimation
- **Statistical accuracy:** More samples = better results
- **Cross-validation:** Multiple independent computations verify correctness
- **Reproducibility:** Random seeds ensure repeatable results

### 4. Software Engineering
- **API design:** BOINC provides clean abstraction layer
- **Error handling:** Robust code handles all failure modes
- **Checkpointing:** Save/restore state for long computations
- **Build systems:** Makefiles, autotools, dependencies

### 5. System Administration
- **Web servers:** Apache configuration
- **Databases:** MySQL setup and management
- **Process management:** Daemons, background processes
- **Networking:** Ports, localhost, client-server communication

---

## Extending the Project

### Ideas for Student Projects

1. **Different Applications**
   - Mandelbrot set generation
   - Prime number search
   - Protein folding simulation
   - Genetic algorithms

2. **Enhanced Features**
   - Web-based result visualization
   - Real-time progress dashboard
   - Email notifications
   - Multi-platform support (Windows, Mac)

3. **Performance Optimization**
   - GPU acceleration
   - Better algorithms (Chudnovsky for PI)
   - Work scheduling optimization
   - Load balancing

4. **Research Projects**
   - Compare different random number generators
   - Analyze cross-validation effectiveness
   - Study fault tolerance under various failure modes
   - Benchmark different hardware configurations

---

## Further Resources

### Official BOINC Documentation
- **Main site:** https://boinc.berkeley.edu/
- **Developer docs:** https://github.com/BOINC/boinc/wiki
- **API reference:** https://boinc.berkeley.edu/trac/wiki/ProjectMain

### Scientific Computing
- **Monte Carlo methods:** https://en.wikipedia.org/wiki/Monte_Carlo_method
- **PI calculation:** https://en.wikipedia.org/wiki/Approximations_of_%CF%80
- **Distributed computing:** https://en.wikipedia.org/wiki/Distributed_computing

### Example BOINC Projects
- **SETI@home:** Search for extraterrestrial intelligence
- **Folding@home:** Protein folding for medical research
- **Rosetta@home:** Protein structure prediction
- **Einstein@Home:** Gravitational wave detection

---

## Credits and License

**Created by:** Educational tutorial for distributed computing students
**Based on:** BOINC 8.3.0 (https://github.com/BOINC/boinc)
**License:** This tutorial and code are provided for educational purposes.

### Component Licenses
- **BOINC:** LGPL 3.0
- **Tutorial code:** MIT License (free to use and modify)

### Acknowledgments
- BOINC development team at UC Berkeley
- Open source community contributors

---

## Support

For questions or issues:
1. Check the Troubleshooting section above
2. Review BOINC documentation
3. Search GitHub issues: https://github.com/BOINC/boinc/issues

---

**Last Updated:** January 2026
**Tutorial Version:** 1.0
