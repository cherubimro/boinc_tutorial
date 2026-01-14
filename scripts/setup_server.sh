#!/bin/bash
#
# BOINC PI Tutorial - Server Setup Script
#
# This script sets up the complete BOINC server including:
# - MySQL database
# - Project structure
# - Apache configuration
# - PI computation application deployment
#

set -e

echo "======================================================================"
echo "BOINC PI Tutorial - Server Setup"
echo "======================================================================"
echo ""

# Configuration
PROJECT_NAME="${PROJECT_NAME:-pi_test}"
DB_USER="boincadm"
DB_PASS="${DB_PASS:-boinc123}"
BOINC_DIR="$HOME/boinc_source"
PROJECT_DIR="$HOME/boinc_test/projects"
TUTORIAL_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "Configuration:"
echo "  Project name: $PROJECT_NAME"
echo "  Database user: $DB_USER"
echo "  Database password: $DB_PASS"
echo "  BOINC source: $BOINC_DIR"
echo "  Project directory: $PROJECT_DIR"
echo ""

# Check BOINC is built
if [ ! -f "$BOINC_DIR/tools/create_work" ]; then
    echo "ERROR: BOINC not built. Run ./build_boinc.sh first"
    exit 1
fi

# Create MySQL database
echo "Setting up MySQL database..."
read -sp "Enter MySQL root password: " MYSQL_ROOT_PASS
echo ""

mysql -u root -p"$MYSQL_ROOT_PASS" << EOF
CREATE DATABASE IF NOT EXISTS $PROJECT_NAME;
CREATE USER IF NOT EXISTS '$DB_USER'@'localhost' IDENTIFIED BY '$DB_PASS';
GRANT ALL PRIVILEGES ON $PROJECT_NAME.* TO '$DB_USER'@'localhost';
FLUSH PRIVILEGES;
EOF

echo "Database created successfully."
echo ""

# Create project structure
echo "Creating project structure..."
mkdir -p "$PROJECT_DIR"
cd "$BOINC_DIR/tools"

./make_project --url_base http://127.0.0.1/${PROJECT_NAME} --db_user $DB_USER --db_passwd $DB_PASS --project_host 192 $PROJECT_NAME

# Move project to our directory
if [ -d "$HOME/$PROJECT_NAME" ]; then
    mv "$HOME/$PROJECT_NAME" "$PROJECT_DIR/"
fi

PROJECT_PATH="$PROJECT_DIR"
cd "$PROJECT_PATH"

echo ""
echo "Building PI computation application..."
cd "$TUTORIAL_DIR/src"
make clean
make

echo ""
echo "Deploying application..."
mkdir -p "$PROJECT_PATH/apps/pi_compute/1.0/x86_64-pc-linux-gnu"
cp pi_compute_1.0_x86_64-pc-linux-gnu "$PROJECT_PATH/apps/pi_compute/1.0/x86_64-pc-linux-gnu/"

# Create templates
cat > "$PROJECT_PATH/templates/pi_in.xml" << 'EOF'
<file_info>
    <number>0</number>
</file_info>
<workunit>
    <file_ref>
        <file_number>0</file_number>
        <open_name>in</open_name>
    </file_ref>
    <rsc_fpops_est>1000000000000</rsc_fpops_est>
    <rsc_fpops_bound>10000000000000</rsc_fpops_bound>
    <rsc_memory_bound>100000000</rsc_memory_bound>
    <rsc_disk_bound>100000000</rsc_disk_bound>
    <delay_bound>86400</delay_bound>
</workunit>
EOF

cat > "$PROJECT_PATH/templates/pi_out.xml" << 'EOF'
<file_info>
    <name><OUTFILE_0/></name>
    <generated_locally/>
    <upload_when_present/>
    <max_nbytes>100000</max_nbytes>
    <url><UPLOAD_URL/></url>
</file_info>
<result>
    <file_ref>
        <file_name><OUTFILE_0/></file_name>
        <open_name>out</open_name>
    </file_ref>
</result>
EOF

# Register application
echo ""
echo "Registering application in database..."
mysql -u $DB_USER -p"$DB_PASS" $PROJECT_NAME << EOF
INSERT IGNORE INTO app (name, user_friendly_name, create_time)
VALUES ('pi_compute', 'PI Computation', UNIX_TIMESTAMP());
INSERT IGNORE INTO platform (name, user_friendly_name, deprecated, create_time)
VALUES ('x86_64-pc-linux-gnu', 'Linux x86-64', 0, UNIX_TIMESTAMP());
EOF

cd "$PROJECT_PATH"
bin/update_versions

# Configure Apache
echo ""
echo "Configuring Apache..."
APACHE_CONF="/tmp/${PROJECT_NAME}.conf"

cat > "$APACHE_CONF" << EOF
Alias /${PROJECT_NAME}/download $PROJECT_PATH/download
Alias /${PROJECT_NAME}/stats $PROJECT_PATH/html/stats
Alias /${PROJECT_NAME} $PROJECT_PATH/html/user
ScriptAlias /${PROJECT_NAME}_cgi $PROJECT_PATH/cgi-bin

<Directory "$PROJECT_PATH/cgi-bin">
    Options ExecCGI
    AllowOverride AuthConfig
    Require all granted
</Directory>

<Directory "$PROJECT_PATH/html">
    Options Indexes FollowSymLinks
    AllowOverride All
    Require all granted
</Directory>

<Directory "$PROJECT_PATH/download">
    Options Indexes FollowSymLinks
    AllowOverride All
    Require all granted
</Directory>
EOF

sudo cp "$APACHE_CONF" /etc/apache2/sites-available/${PROJECT_NAME}.conf 2>/dev/null || \
sudo cp "$APACHE_CONF" /etc/apache/conf.d/${PROJECT_NAME}.conf

# Enable site (Debian/Ubuntu)
if command -v a2ensite &> /dev/null; then
    sudo a2ensite ${PROJECT_NAME}.conf
fi

sudo systemctl restart apache2 || sudo systemctl restart httpd

# Fix upload permissions
chmod 777 "$PROJECT_PATH/upload"
chmod 777 "$PROJECT_PATH/download"

# Create get_project_config.xml
cat > "$PROJECT_PATH/html/user/get_project_config.xml" << EOF
<?xml version="1.0" encoding="iso-8859-1"?>
<project_config>
    <name>$PROJECT_NAME</name>
    <master_url>http://127.0.0.1/$PROJECT_NAME/</master_url>
    <web_rpc_url_base>http://127.0.0.1/$PROJECT_NAME/</web_rpc_url_base>
    <scheduler>http://127.0.0.1/${PROJECT_NAME}_cgi/cgi</scheduler>
    <min_passwd_length>6</min_passwd_length>
    <account_manager>0</account_manager>
    <uses_username>0</uses_username>
</project_config>
EOF

# Create custom index.html
cat > "$PROJECT_PATH/html/user/index.html" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>$PROJECT_NAME</title>
    <link rel="boinc_scheduler" href="http://127.0.0.1/${PROJECT_NAME}_cgi/cgi">
</head>
<body>
<h1>BOINC Project: $PROJECT_NAME</h1>
<p>PI Computation Project</p>
</body>
</html>
EOF

# Start daemons
echo ""
echo "Starting BOINC daemons..."
cd "$PROJECT_PATH"
bin/start

# Create initial work units
echo ""
echo "Creating initial work units..."
echo "10000000" > download/wu_10m.txt

bin/create_work --appname pi_compute --wu_name pi_wu_1 \
    --wu_template templates/pi_in.xml \
    --result_template templates/pi_out.xml \
    --target_nresults 3 \
    wu_10m.txt

echo ""
echo "======================================================================"
echo "Server setup complete!"
echo "======================================================================"
echo ""
echo "Server details:"
echo "  Project URL: http://127.0.0.1/$PROJECT_NAME/"
echo "  Scheduler: http://127.0.0.1/${PROJECT_NAME}_cgi/cgi"
echo "  Project directory: $PROJECT_PATH"
echo ""
echo "Next: Run ./start_clients.sh to start BOINC clients"
echo ""
