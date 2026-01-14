#!/bin/bash
#
# BOINC PI Tutorial - Dependency Installation Script for openSUSE/SUSE
#
# This script installs all required dependencies for building and running
# the BOINC server and clients on openSUSE/SUSE-based systems.
#

set -e  # Exit on error

echo "======================================================================"
echo "BOINC PI Tutorial - Installing Dependencies (openSUSE/SUSE)"
echo "======================================================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Please run as root (use sudo)"
    exit 1
fi

echo "Refreshing repositories..."
zypper refresh

echo ""
echo "Installing build tools..."
zypper install -y \
    gcc-c++ \
    make \
    autoconf \
    automake \
    libtool \
    m4 \
    pkg-config \
    git

echo ""
echo "Installing BOINC dependencies..."
zypper install -y \
    libopenssl-devel \
    libcurl-devel \
    libmariadb-devel \
    libnotify-devel \
    libX11-devel \
    libXScrnSaver-devel \
    libxcb-devel \
    Mesa-libGL-devel \
    freeglut-devel \
    libXi-devel \
    libXmu-devel

echo ""
echo "Installing database server..."
zypper install -y \
    mariadb \
    mariadb-client \
    mariadb-tools

echo ""
echo "Installing web server..."
zypper install -y \
    apache2 \
    php8 \
    php8-mysql \
    php8-cli \
    apache2-mod_php8

echo ""
echo "Installing Python dependencies..."
zypper install -y \
    python3 \
    python3-pip \
    python3-PyMySQL

echo ""
echo "Installing additional libraries..."
zypper install -y \
    nlohmann_json-devel \
    libzip-devel \
    systemd-devel

echo ""
echo "Enabling Apache modules..."
a2enmod php8
a2enmod cgi
a2enmod rewrite

echo ""
echo "Starting services..."
systemctl start mariadb
systemctl enable mariadb
systemctl start apache2
systemctl enable apache2

echo ""
echo "Setting up MySQL..."
echo "Please set a root password when prompted."
echo ""
read -p "Press ENTER to continue..."
mysql_secure_installation

echo ""
echo "======================================================================"
echo "Dependency installation complete!"
echo "======================================================================"
echo ""
echo "Next steps:"
echo "  1. Run ./build_boinc.sh to compile BOINC"
echo "  2. Run ./setup_server.sh to configure the server"
echo "  3. Run ./start_clients.sh to start clients"
echo ""
