#!/bin/bash
#
# BOINC PI Tutorial - Dependency Installation Script for Ubuntu/Debian
#
# This script installs all required dependencies for building and running
# the BOINC server and clients on Ubuntu/Debian-based systems.
#

set -e  # Exit on error

echo "======================================================================"
echo "BOINC PI Tutorial - Installing Dependencies (Ubuntu/Debian)"
echo "======================================================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Please run as root (use sudo)"
    exit 1
fi

echo "Updating package lists..."
apt-get update

echo ""
echo "Installing build tools..."
apt-get install -y \
    build-essential \
    g++ \
    make \
    autoconf \
    automake \
    libtool \
    m4 \
    pkg-config \
    git

echo ""
echo "Installing BOINC dependencies..."
apt-get install -y \
    libssl-dev \
    libcurl4-openssl-dev \
    libmysqlclient-dev \
    libnotify-dev \
    libx11-dev \
    libxss-dev \
    libxcb-util0-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    libxi-dev \
    libxmu-dev

echo ""
echo "Installing database server..."
apt-get install -y \
    mariadb-server \
    mariadb-client

echo ""
echo "Installing web server..."
apt-get install -y \
    apache2 \
    php \
    php-mysql \
    php-cli \
    php-xml \
    libapache2-mod-php

echo ""
echo "Installing Python dependencies..."
apt-get install -y \
    python3 \
    python3-pip \
    python3-mysqldb

pip3 install pymysql

echo ""
echo "Installing additional libraries..."
apt-get install -y \
    nlohmann-json3-dev \
    libzip-dev \
    libsystemd-dev

echo ""
echo "Enabling Apache modules..."
a2enmod cgi
a2enmod rewrite

echo ""
echo "Starting services..."
systemctl start mariadb
systemctl enable mariadb
systemctl start apache2
systemctl enable apache2

echo ""
echo "Running MySQL secure installation..."
echo "Please set a root password when prompted."
echo "Answer 'Y' to all security questions."
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
