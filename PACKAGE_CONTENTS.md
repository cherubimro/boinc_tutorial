# BOINC PI Tutorial - Complete Package Contents

**Package:** `boinc_pi_tutorial_complete.tar.gz`
**Size:** 33 KB (compressed)
**Created:** January 12, 2026

---

## Directory Structure

```
boinc_tutorial/
├── README.md                       # Main tutorial (60+ pages)
├── QUICKSTART.md                   # Fast-track guide (30 minutes)
├── INSTRUCTOR_NOTES.md             # Teaching guide for educators
├── TESTED_STATUS.md                # HONEST testing status - READ THIS FIRST
├── LICENSE                         # MIT License
├── PACKAGE_CONTENTS.md             # This file
│
├── src/                            # Source code
│   ├── pi_compute.cpp              # PI computation application with BOINC API
│   └── Makefile                    # Build configuration
│
├── scripts/                        # Automated setup scripts
│   ├── install_dependencies_debian.sh     # Ubuntu/Debian packages (UNTESTED)
│   ├── install_dependencies_opensuse.sh   # openSUSE packages (TESTED ✓)
│   ├── build_boinc.sh              # Build BOINC from source
│   ├── setup_server.sh             # Complete server setup
│   ├── start_clients.sh            # Start 3 test clients
│   └── check_results.sh            # View computation results
│
├── sql/                            # Database scripts
│   ├── README.md                   # SQL documentation
│   ├── create_database.sql         # Create database and user
│   ├── register_application.sql    # Register PI app and platform
│   └── check_status.sql            # Status monitoring queries
│
├── templates/                      # BOINC XML templates
│   ├── README.md                   # Template documentation
│   ├── pi_in.xml                   # Input work unit template
│   ├── pi_out.xml                  # Output result template
│   └── version.xml                 # Application version descriptor
│
├── config/                         # Configuration files
│   ├── README.md                   # Configuration documentation
│   ├── apache_pi_test.conf         # Apache web server config
│   └── get_project_config.xml      # BOINC project config
│
└── examples/                       # Example files
    ├── README.md                   # Examples documentation
    ├── example_input.txt           # Sample input (1B iterations)
    └── example_output.txt          # Sample output with explanation
```

---

## File Descriptions

### Documentation Files

#### README.md (Main Tutorial)
- **Purpose:** Comprehensive BOINC tutorial
- **Length:** 60+ pages
- **Contents:**
  - Complete architecture explanation
  - Step-by-step installation
  - BOINC API reference
  - Monte Carlo method explanation
  - Troubleshooting guide
  - Extension ideas
- **Audience:** Students and self-learners

#### QUICKSTART.md
- **Purpose:** Fast-track setup guide
- **Time:** 30-45 minutes for experienced users
- **Contents:**
  - Condensed installation steps
  - Quick reference commands
  - Minimal explanations
- **Audience:** Users already familiar with BOINC concepts

#### INSTRUCTOR_NOTES.md
- **Purpose:** Teaching guide
- **Contents:**
  - Course integration suggestions
  - Common student questions & answers
  - Exercise ideas (easy/medium/advanced)
  - Assessment rubrics
  - Troubleshooting tips for instructors
- **Audience:** Educators teaching distributed systems

#### TESTED_STATUS.md ⚠️ READ THIS FIRST
- **Purpose:** Honest testing disclosure
- **Contents:**
  - What was actually tested (openSUSE Leap 15.4 only)
  - What has NOT been tested (Ubuntu, WSL2, etc.)
  - Student responsibilities
  - Expected problems and learning approach
- **Importance:** CRITICAL - sets realistic expectations

#### LICENSE
- **Type:** MIT License
- **Permissions:** Free to use, modify, distribute
- **Requirements:** Attribution appreciated

---

### Source Code

#### src/pi_compute.cpp
- **Lines:** 300+
- **Language:** C++
- **Features:**
  - Monte Carlo PI estimation algorithm
  - BOINC API integration
  - Fault-tolerant checkpointing
  - `/dev/urandom` random seeding (critical for cross-validation)
  - Progress reporting
  - Comprehensive error handling
  - Detailed comments for learning
- **Key Functions:**
  - `compute_pi()` - Monte Carlo simulation
  - `write_checkpoint()` - Save computation state
  - `read_checkpoint()` - Resume from checkpoint
  - Main loop with BOINC API calls

#### src/Makefile
- **Purpose:** Build configuration
- **Compiler:** g++
- **Flags:** `-O3 -Wall` for optimization and warnings
- **Libraries:** BOINC client library, pthreads
- **Outputs:**
  - `pi_compute` - Standalone executable
  - `pi_compute_1.0_x86_64-pc-linux-gnu` - BOINC deployment binary

---

### Scripts (All Executable)

#### install_dependencies_debian.sh
- **Status:** ⚠️ UNTESTED - provided as starting point
- **Target:** Ubuntu 20.04+, Debian 11+
- **Packages:** gcc, make, autotools, MySQL, Apache, PHP, JSON libs
- **Note:** Students must adapt to their specific distribution

#### install_dependencies_opensuse.sh
- **Status:** ✅ TESTED on openSUSE Leap 15.4
- **Target:** openSUSE Leap, SUSE Linux Enterprise
- **Packages:** Same as Debian but using `zypper`
- **Note:** Only guaranteed to work on openSUSE Leap 15.4

#### build_boinc.sh
- **Purpose:** Build BOINC from source
- **Version:** 8.3.0 (client-release-8.3 branch)
- **Time:** 10-15 minutes
- **Steps:**
  1. Clone BOINC from GitHub
  2. Run autotools setup
  3. Configure (server + client)
  4. Build with make
  5. Verify binaries

#### setup_server.sh
- **Purpose:** Complete server configuration
- **Interactive:** Asks for MySQL root password
- **Steps:**
  1. Create MySQL database and user
  2. Run BOINC `make_project` tool
  3. Compile PI application
  4. Deploy application to project
  5. Create XML templates
  6. Register application in database
  7. Configure Apache web server
  8. Start BOINC daemons
  9. Create initial work units
- **Time:** 5-10 minutes

#### start_clients.sh
- **Purpose:** Start 3 BOINC clients on localhost
- **Configuration:**
  - Client 1: Port 31417, ~/boinc_clients/client1/
  - Client 2: Port 31418, ~/boinc_clients/client2/
  - Client 3: Port 31419, ~/boinc_clients/client3/
- **Actions:**
  1. Create client directories
  2. Attach clients to project
  3. Start clients in background
  4. Show status
- **Time:** 2-3 minutes

#### check_results.sh
- **Purpose:** View computation results
- **Shows:**
  - All work units
  - Result files from each client
  - PI values and accuracy
  - Cross-validation status
- **Usage:** Run periodically to monitor progress

---

### SQL Scripts

#### create_database.sql
- **Purpose:** Database initialization
- **Creates:**
  - `pi_test` database
  - `boincadm` user (password: `boinc123`)
  - Grants appropriate privileges
- **Usage:** `mysql -u root -p < create_database.sql`
- **Security:** ⚠️ Change password for production!

#### register_application.sql
- **Purpose:** Register PI app in BOINC
- **Inserts:**
  - Application: `pi_compute`
  - Platform: `x86_64-pc-linux-gnu`
- **Usage:** `mysql -u boincadm -pboinc123 pi_test < register_application.sql`
- **Note:** Embedded in setup_server.sh

#### check_status.sql
- **Purpose:** Status monitoring queries
- **Queries:**
  - List all work units
  - List all results
  - Count by status
  - Show connected clients (hosts)
  - Result validation status
- **Usage:** Interactive or scripted monitoring

---

### Templates (BOINC XML)

#### pi_in.xml - Input Template
- **Purpose:** Defines work unit structure
- **Elements:**
  - File reference (input file)
  - Resource estimates (FPOPS, memory, disk)
  - Time bounds (24 hour deadline)
- **Used by:** `create_work` command
- **Location on server:** `~/boinc_test/projects/templates/`

#### pi_out.xml - Output Template
- **Purpose:** Defines result structure
- **Elements:**
  - Output file naming
  - Upload triggers
  - Size limits (100KB max)
  - Upload URL (auto-filled)
- **Used by:** BOINC scheduler
- **Location on server:** `~/boinc_test/projects/templates/`

#### version.xml - Application Version
- **Purpose:** Application binary metadata
- **Elements:**
  - Physical filename
  - Main program flag
- **Location on server:** `~/boinc_test/projects/apps/pi_compute/1.0/x86_64-pc-linux-gnu/`
- **Used by:** `update_versions` command

---

### Configuration Files

#### apache_pi_test.conf
- **Purpose:** Apache virtual host config
- **Aliases:**
  - `/pi_test/` → Project HTML
  - `/pi_test_cgi/` → Scheduler CGI
  - `/pi_test/download/` → Application binaries
- **Permissions:** CGI execution, directory access
- **Note:** Must adjust paths from `/home/bu` to your home directory

#### get_project_config.xml
- **Purpose:** Client project discovery
- **Contains:**
  - Project name and URLs
  - Scheduler endpoint
  - Account policies
- **Location:** `~/boinc_test/projects/html/user/`
- **Accessed by:** BOINC clients during attachment

---

### Examples

#### example_input.txt
- **Content:** `1000000000` (1 billion iterations)
- **Purpose:** Test input for standalone execution
- **Usage:** `./pi_compute < example_input.txt`

#### example_output.txt
- **Purpose:** Shows expected output format
- **Contains:**
  - PI estimate
  - Error calculation
  - Accuracy percentage
  - Explanation of variation

---

## What's Included vs. What's Generated

### ✅ Included in Package
- All source code
- All scripts (executable)
- All documentation
- All SQL scripts
- All XML templates
- All configuration examples

### ⚠️ NOT Included (Generated During Setup)
- BOINC binaries (built from source via `build_boinc.sh`)
- MySQL database (created by `setup_server.sh`)
- Project directory structure (created by BOINC `make_project`)
- Compiled PI application (built via `make`)
- Apache config (installed by `setup_server.sh`)
- Client directories (created by `start_clients.sh`)

---

## Installation Order

Students should use files in this order:

1. **TESTED_STATUS.md** - Understand testing status
2. **README.md** or **QUICKSTART.md** - Choose learning path
3. **install_dependencies_*.sh** - Install system packages
4. **build_boinc.sh** - Build BOINC from source
5. **src/Makefile** - Compile PI application
6. **setup_server.sh** - Configure server (uses SQL scripts, templates, configs)
7. **start_clients.sh** - Start test clients
8. **check_results.sh** - Monitor results

---

## Key Features

### ✅ Actually Tested (openSUSE Leap 15.4)
- Complete BOINC server setup
- 3 clients processing work units
- Cross-validation with different PI values
- `/dev/urandom` random seeding
- Apache CGI scheduler
- MySQL database integration
- Checkpoint/resume functionality

### ⚠️ Provided But NOT Tested
- Ubuntu/Debian installation
- WSL2 setup
- Fedora/RHEL installation
- Network-based distributed setup (multiple machines)
- Extensions and modifications

---

## Student Expectations

### Students MUST:
1. Test everything on their own system
2. Debug distribution-specific issues
3. Adapt scripts to their environment
4. Understand the code before running
5. Modify the system (compute e, fibonacci, etc.)
6. Document their changes

### This is NOT:
- Production-ready software
- Guaranteed to work on all systems
- A turn-key solution
- Tested on multiple platforms

### This IS:
- A working example on ONE system (openSUSE)
- Educational code with explanations
- A starting point for learning
- Real distributed computing concepts

---

## Success Criteria

Students succeed when they can:
- Explain WHY each component is necessary
- Fix problems independently
- Modify to compute something different
- Explain distributed computing concepts demonstrated

---

## Support Resources

### Included Documentation
- README.md - Comprehensive tutorial
- QUICKSTART.md - Fast reference
- INSTRUCTOR_NOTES.md - Teaching guide
- 4 × README.md in subdirectories (sql/, templates/, config/, examples/)

### External Resources
- BOINC Website: https://boinc.berkeley.edu/
- BOINC GitHub: https://github.com/BOINC/boinc
- BOINC Wiki: https://github.com/BOINC/boinc/wiki

---

## Reproducibility

**Tested Configuration:**
- OS: openSUSE Leap 
- Setup: 1 server + 3 clients on localhost (127.0.0.1)
- Date: January 12, 2026

**Verified Results:**
```
Work Unit: pi_wu_17 (10M iterations)
  Client A: PI = 3.141630400 (99.9988% accuracy)
  Client B: PI = 3.141109600 (99.9846% accuracy)
  Client C: PI = 3.141212000 (99.9879% accuracy)

Status: ✓ All different values (cross-validation working)
```

---

## Changes from Original Package

Previous `boinc_pi_tutorial.tar.gz` (21 KB) was missing:
- ❌ SQL scripts directory was empty
- ❌ XML templates not included
- ❌ Apache config not included
- ❌ Example input/output files missing
- ❌ Configuration files directory missing
- ❌ Resource bounds in templates

This complete package (28 KB) includes:
- ✅ 3 SQL scripts with documentation
- ✅ 3 XML templates with documentation
- ✅ Apache config with documentation
- ✅ Example files with documentation
- ✅ Complete config/ directory
- ✅ Resource bounds in pi_in.xml
- ✅ Comprehensive README.md in each subdirectory

---

**Package is now complete and ready for distribution.**
