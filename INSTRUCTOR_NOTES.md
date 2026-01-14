# BOINC PI Tutorial - Instructor Notes

## Teaching Guide for Educators

This document provides guidance for instructors using the BOINC PI Tutorial in a classroom or workshop setting.

---

## Course Integration

### Suitable Courses
- Distributed Systems 
- Parallel and Distributed Computing
- Cloud Computing
- Scientific Computing
- Software Engineering (advanced topics)

### Prerequisites
Students should have:
- Intermediate C/C++ programming skills
- Basic Linux command-line knowledge
- Understanding of client-server architecture
- Familiarity with databases (helpful but not required)

---

## Learning Objectives by Bloom's Taxonomy

### Remember (Knowledge)
- Define distributed computing terminology
- List BOINC system components
- Identify Monte Carlo method steps

### Understand (Comprehension)
- Explain client-server communication flow
- Describe checkpointing mechanism
- Summarize cross-validation purpose

### Apply (Application)
- Build BOINC system from source
- Configure server and clients
- Create new work units

### Analyze (Analysis)
- Compare different random seeding methods
- Examine why results vary between clients
- Debug system issues using logs

### Evaluate (Synthesis)
- Assess system performance
- Propose improvements
- Design validation strategies

### Create (Evaluation)
- Implement new BOINC applications
- Extend PI computation algorithm
- Develop custom validation methods

---

## Common Student Questions & Answers

### Q1: "Why do we need distributed computing for PI calculation?"
**A:** We don't actually need it for PI - this is a pedagogical example. The real value is learning the distributed computing concepts that apply to genuinely large problems (protein folding, climate modeling, etc.). PI calculation is simple enough to understand but complex enough to demonstrate all the key concepts.

### Q2: "Why do clients sometimes get the same PI value?"
**A:** This is actually a great teaching moment! With Monte Carlo simulation and limited precision (5-10M samples), coincidental identical results are possible. This demonstrates:
- The nature of random sampling
- Why we need cross-validation
- The importance of random seeding
- Statistical variance vs. deterministic errors

### Q3: "Why does BOINC see all 3 local clients as one host?"
**A:** BOINC uses hardware signatures (CPU ID, MAC address) for host identification. Since all 3 clients run on the same physical machine, BOINC merges them. In production, clients would be on different physical machines. This is expected behavior for local testing.

### Q4: "Can we run this on Windows?"
**A:** Yes, via WSL2 (Windows Subsystem for Linux). The installation script for Ubuntu/Debian works in WSL2. Native Windows BOINC server setup is much more complex and not covered in this tutorial.

### Q5: "Why build BOINC from source instead of using packages?"
**A:** Several reasons:
- Educational value (understand the build process)
- System packages often lack server components
- Version consistency (tutorial tested with 8.3.0)
- Bug workarounds (e.g., client segfault in 7.18.1)

---

## Key Teaching Points

### 1. Architecture Patterns
**Emphasize:** The classic distributed computing architecture:
- **Centralized work queue** (server database)
- **Worker pool** (clients)
- **Result aggregation** (scheduler)
- **Validation** (cross-checking)

**Real-world examples:** MapReduce, Kubernetes jobs, rendering farms

### 2. Fault Tolerance
**Demonstrate:** Interrupt a client mid-computation (Ctrl+C) then restart it.
The checkpoint mechanism allows it to resume exactly where it left off.

**Discussion:** What happens if a client never reports back? (Timeouts, work reassignment)

### 3. Random Number Generation
**Important concept:** The evolution from `time()` → `gettimeofday() + getpid()` → `/dev/urandom` shows progressively better randomness sources.

**Exercise:** Have students run the same work unit multiple times and observe result variation.

### 4. Validation Strategies
**Compare approaches:**
- **Redundant computation:** 3 clients compute same task (current method)
- **Quorum voting:** Majority wins
- **Trusted clients:** Some clients more reliable than others
- **Result verification:** Mathematical proof of correctness

---

## Suggested Exercises

### Exercise 1: Modify Iteration Count (Easy)
**Goal:** Create work units with different iteration counts (1M, 10M, 100M)
**Learning:** Understand accuracy vs. computation time tradeoff
**Time:** 15 minutes

```bash
# Create 100M iteration work unit
echo "100000000" > ~/boinc_test/projects/download/wu_100m.txt
cd ~/boinc_test/projects
bin/create_work --appname pi_compute --wu_name pi_wu_large \
    --wu_template templates/pi_in.xml \
    --result_template templates/pi_out.xml \
    --target_nresults 3 wu_100m.txt
```

### Exercise 2: Add Progress Reporting (Medium)
**Goal:** Modify `pi_compute.cpp` to report progress every 1M iterations
**Learning:** BOINC API usage, code modification

**Hint:** Use `fprintf(stderr, ...)` for status messages that appear in logs.

### Exercise 3: Implement New Algorithm (Advanced)
**Goal:** Replace Monte Carlo with Chudnovsky algorithm or Leibniz formula
**Learning:** Algorithm design, maintaining BOINC compatibility

### Exercise 4: Build Performance Dashboard (Project)
**Goal:** Create web page showing real-time results
**Learning:** Full-stack integration (database, backend, frontend)

---

## Troubleshooting Guide for Instructors

### Issue: Students can't install dependencies
**Cause:** Usually missing sudo privileges or wrong distribution
**Solution:**
- Provide VM images with dependencies pre-installed
- Use Docker container (advanced)
- Pair students (one with admin rights helps others)

### Issue: BOINC build fails
**Common causes:**
1. Missing dependencies - re-run install script
2. Parallel build race condition - use `make -j1`
3. Disk space - need 5GB for build artifacts

**Solution:** Provide pre-built BOINC binaries for time-constrained labs

### Issue: Clients don't get work
**Checklist:**
1. Are daemons running? `ps aux | grep -E "feeder|transitioner"`
2. Is Apache running? `systemctl status apache2`
3. Are work units in database? `mysql -u boincadm -pboinc123 pi_test -e "SELECT * FROM workunit"`
4. Check scheduler log: `tail -f ~/boinc_test/projects/log_192/scheduler.log`

### Issue: All results identical
**Cause:** Old binary without `/dev/urandom` fix
**Solution:** Recompile pi_compute.cpp and redeploy with `bin/update_versions`

---

## Extension Ideas for Advanced Students

### 1. GPU Acceleration
Modify pi_compute.cpp to use CUDA or OpenCL for GPU computation.
**Learning:** Heterogeneous computing, GPU programming

### 2. Docker Deployment
Containerize the entire system for easy deployment.
**Learning:** Containerization, orchestration

### 3. Cloud Deployment
Deploy server on AWS/Azure, clients on multiple VMs.
**Learning:** Cloud computing, networking

### 4. Machine Learning Integration
Use BOINC to distribute neural network training.
**Learning:** ML + distributed systems

### 5. Real Scientific Application
- Prime number search
- Protein folding (simplified)
- Mandelbrot set generation
- Climate modeling simulation

---

## Class Discussion Topics

1. **Ethics of Volunteer Computing**
   - Should volunteers be compensated?
   - Privacy concerns with distributed computation
   - Environmental impact of distributed computing

2. **Alternative Architectures**
   - Compare BOINC vs. Hadoop vs. Kubernetes
   - Centralized vs. peer-to-peer approaches
   - When to use each architecture

3. **Trust and Security**
   - How to prevent malicious results?
   - What if volunteers return fake data?
   - Code signing and sandboxing

4. **Scalability**
   - How does BOINC scale to millions of clients?
   - Database bottlenecks
   - Network bandwidth requirements

---

## Contact for Support

If you encounter issues teaching with this tutorial:
1. Check the README.md troubleshooting section
2. Review BOINC documentation
3. Search GitHub issues: https://github.com/BOINC/boinc/issues

---
