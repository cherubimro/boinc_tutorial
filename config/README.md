# BOINC Configuration Files

This directory contains configuration files for Apache and BOINC project.

## Files

### apache_pi_test.conf
Apache web server configuration for the BOINC project.

**Installation on Debian/Ubuntu:**
```bash
sudo cp apache_pi_test.conf /etc/apache2/sites-available/pi_test.conf
sudo a2ensite pi_test.conf
sudo systemctl restart apache2
```

**Installation on openSUSE/SUSE:**
```bash
sudo cp apache_pi_test.conf /etc/apache2/conf.d/pi_test.conf
sudo systemctl restart apache2
```

**What it configures:**
- `/pi_test/` - Main project web interface
- `/pi_test_cgi/` - CGI scheduler endpoint
- `/pi_test/download/` - Work unit and application downloads
- `/pi_test/stats/` - Statistics pages
- CGI execution permissions
- Directory access permissions

**Important:** You MUST change the paths from `/home/bu` to your actual home directory!

### get_project_config.xml
BOINC project configuration returned to clients.

**Installation:**
```bash
cp get_project_config.xml ~/boinc_test/projects/html/user/
```

**What it contains:**
- Project name
- Master URL (project home page)
- Scheduler URL (where clients get work)
- Account policies (password length, username requirements)

The setup script automatically creates this file with correct URLs.

## URL Structure

BOINC uses specific URL patterns:

- **Master URL**: `http://127.0.0.1/pi_test/`
  - Project home page
  - Clients use this to attach to project

- **Scheduler URL**: `http://127.0.0.1/pi_test_cgi/cgi`
  - Work distribution endpoint
  - Clients contact this to get work units
  - Must be CGI script with execute permissions

- **Download URL**: `http://127.0.0.1/pi_test/download/`
  - Application binaries
  - Input files for work units
  - Auto-configured by BOINC

## Apache Requirements

### Required Apache Modules:

**Debian/Ubuntu:**
```bash
sudo a2enmod cgi
sudo a2enmod alias
sudo systemctl restart apache2
```

**openSUSE:**
```bash
# Usually enabled by default
# Check with: httpd -M | grep cgi
```

### CGI Execution

The scheduler is a CGI binary at:
```
~/boinc_test/projects/cgi-bin/cgi
```

Apache must have:
- Execute permissions on CGI scripts
- CGI module enabled
- Correct ScriptAlias directive

### Troubleshooting Apache

**Check Apache is running:**
```bash
sudo systemctl status apache2  # Debian/Ubuntu/SUSE
# or
sudo systemctl status httpd    # RHEL/Fedora
```

**Test URLs:**
```bash
# Should show project page
curl http://127.0.0.1/pi_test/

# Should return XML
curl http://127.0.0.1/pi_test/get_project_config.xml

# Should execute (may return error, but shouldn't 404)
curl http://127.0.0.1/pi_test_cgi/cgi
```

**Check Apache logs:**
```bash
# Debian/Ubuntu
tail -f /var/log/apache2/error.log

# openSUSE
tail -f /var/log/apache2/error_log

# RHEL/Fedora
tail -f /var/log/httpd/error_log
```

**Common errors:**

- **403 Forbidden** - Permission issue, check directory permissions
- **404 Not Found** - Wrong path in config, check Alias/ScriptAlias
- **500 Internal Server Error** - CGI execution problem, check execute bit
- **Connection refused** - Apache not running

## Security Considerations

**For testing/learning (current setup):**
- All access from localhost (127.0.0.1)
- Default passwords (boinc123)
- No HTTPS
- Upload directories world-writable (chmod 777)

**For production (DO NOT use this tutorial as-is):**
- Use real domain name
- Enable HTTPS with SSL certificates
- Strong database passwords
- Restrict upload directory permissions
- Enable BOINC account system
- Configure firewall rules
- Use BOINC's built-in security features

This tutorial is for EDUCATIONAL purposes on localhost only!

## File Permissions

BOINC needs specific permissions:

```bash
# Upload directory (results from clients)
chmod 777 ~/boinc_test/projects/upload

# Download directory (apps and input files)
chmod 755 ~/boinc_test/projects/download

# CGI binaries
chmod 755 ~/boinc_test/projects/cgi-bin/cgi

# Make sure Apache can read html directory
chmod 755 ~/boinc_test/projects/html
```

The setup script handles this automatically.
