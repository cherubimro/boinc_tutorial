# BOINC PI Tutorial - Quick Start Guide

## For Impatient Students 🚀

Get a BOINC distributed computing system running in 30 minutes!

### Prerequisites

- Linux system (Ubuntu 20.04+, openSUSE 15.4+, Debian 11+, or WSL2)
- 8 GB RAM minimum
- 10 GB disk space
- Internet connection (for downloading dependencies and BOINC source)

### Step-by-Step Commands

```bash
# 1. Extract the archive
tar -xzf boinc_pi_tutorial.tar.gz
cd boinc_pi_tutorial

# 2. Install dependencies (choose your distribution)
# For Ubuntu/Debian:
sudo ./scripts/install_dependencies_debian.sh

# For openSUSE/SUSE:
sudo ./scripts/install_dependencies_opensuse.sh

# 3. Build BOINC (takes ~15 minutes)
./scripts/build_boinc.sh

# 4. Compile PI application
cd src
make
cd ..

# 5. Setup server (will prompt for MySQL root password)
./scripts/setup_server.sh

# 6. Start 3 clients
./scripts/start_clients.sh

# 7. Check if it's working
./scripts/check_results.sh
```

### What Should Happen

After a few minutes, you should see:
- 3 BOINC clients running
- Work units being downloaded
- PI calculations completing
- Results being uploaded

### Verify It's Working

```bash
# Check clients are running
ps aux | grep boinc_client

# Check logs
tail -f ~/boinc_clients/client_a/client_a.log

# Check database
mysql -u boincadm -pboinc123 pi_test -e "SELECT COUNT(*) FROM result WHERE server_state=5"
```

### Expected Results

Within 5-10 minutes you should see results like:

```
Work Unit: pi_wu_1
  Client A: PI = 3.141630 (99.998% accuracy)
  Client B: PI = 3.141110 (99.984% accuracy)
  Client C: PI = 3.141212 (99.987% accuracy)
```

All three values will be **different** (proving cross-validation works).

### Troubleshooting

**Clients not getting work?**
```bash
cd ~/boinc_test/projects
echo "10000000" > download/test.txt
bin/create_work --appname pi_compute --wu_name test_wu --wu_template templates/pi_in.xml --result_template templates/pi_out.xml --target_nresults 3 test.txt
```

**Permission errors?**
```bash
chmod 777 ~/boinc_test/projects/upload
chmod 777 ~/boinc_test/projects/download
```

**MySQL errors?**
```bash
mysql -u root -p << 'EOF'
GRANT ALL PRIVILEGES ON pi_test.* TO 'boincadm'@'localhost';
FLUSH PRIVILEGES;
EOF
```

### Next Steps

Once it's working:
1. Read the full [README.md](README.md) to understand the code
2. Modify `src/pi_compute.cpp` to experiment
3. Create your own work units with different iteration counts
4. Study the BOINC architecture

### Getting Help

- Check `README.md` for detailed explanations
- Look at log files in `~/boinc_test/projects/log_192/`
- Review client logs in `~/boinc_clients/client_*/`
- Search BOINC documentation: https://boinc.berkeley.edu/

---

**Time to complete:** 30-45 minutes
**Difficulty:** Intermediate
**Learning value:** High 🎓
