# BOINC SQL Scripts

This directory contains SQL scripts for database setup and management.

## Files

### create_database.sql
Creates the MySQL/MariaDB database and user account for BOINC.

**Usage:**
```bash
mysql -u root -p < create_database.sql
```

**What it does:**
- Creates `pi_test` database
- Creates `boincadm` user with password `boinc123`
- Grants all privileges on the database
- Shows verification output

**Security Note:** Change the password `boinc123` in both this file and `setup_server.sh` for production use.

### register_application.sql
Registers the PI computation application and platform in the BOINC database.

**Usage:**
```bash
mysql -u boincadm -pboinc123 pi_test < register_application.sql
```

**What it does:**
- Inserts `pi_compute` application into `app` table
- Registers `x86_64-pc-linux-gnu` platform
- Shows registered applications and platforms

**Note:** Run this AFTER `make_project` creates the database schema. The setup script does this automatically.

### check_status.sql
Collection of useful SQL queries to check server status.

**Usage:**
```bash
# Run all queries
mysql -u boincadm -pboinc123 pi_test < check_status.sql

# Or run individual queries interactively
mysql -u boincadm -pboinc123 pi_test
```

**Queries included:**
- Show all work units
- Show all results
- Show detailed result info with hostnames
- Count work units by status
- Count results by state
- Show registered clients (hosts)

## Database Schema

The BOINC database schema is created by `make_project` tool. Key tables:

- **app** - Applications (e.g., pi_compute)
- **platform** - CPU/OS platforms (e.g., x86_64-pc-linux-gnu)
- **app_version** - Specific versions of apps for platforms
- **workunit** - Work units to be computed
- **result** - Individual results (one WU -> multiple results for redundancy)
- **host** - Client computers
- **user** - User accounts (if using account system)

## Manual Database Operations

### View all work units:
```sql
SELECT id, name, create_time, canonical_resultid
FROM workunit ORDER BY create_time DESC;
```

### View results for a specific work unit:
```sql
SELECT id, name, server_state, outcome, cpu_time
FROM result WHERE workunitid = 1;
```

### Check daemon status in database:
```sql
SELECT name, pid, time FROM daemon WHERE time > UNIX_TIMESTAMP() - 300;
```

### Delete all work (DANGEROUS - testing only):
```sql
DELETE FROM result;
DELETE FROM workunit;
```

## Common Server States

Results go through these states:

- **server_state = 2** - UNSENT (waiting to be sent to client)
- **server_state = 4** - IN_PROGRESS (sent to client, computing)
- **server_state = 5** - OVER (computation complete)

## Troubleshooting

### No work being sent to clients:
```sql
SELECT COUNT(*) FROM workunit WHERE canonical_resultid = 0;
SELECT COUNT(*) FROM result WHERE server_state = 2;
```

If both return 0, create more work with `bin/create_work`.

### Results stuck in IN_PROGRESS:
Check if clients crashed. Results eventually timeout and get reassigned.

### Platform not supported error:
```sql
SELECT * FROM platform;
INSERT INTO platform (name, user_friendly_name, deprecated, create_time)
VALUES ('x86_64-pc-linux-gnu', 'Linux x86-64', 0, UNIX_TIMESTAMP());
```

Then run `bin/update_versions`.
