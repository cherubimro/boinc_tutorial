#!/bin/bash
#
# BOINC PI Tutorial - Build BOINC from Source
#

set -e

echo "======================================================================"
echo "Building BOINC from Source"
echo "======================================================================"
echo ""

BOINC_DIR="$HOME/boinc_source"

if [ -d "$BOINC_DIR" ]; then
    echo "BOINC source directory already exists at $BOINC_DIR"
    read -p "Remove and re-clone? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$BOINC_DIR"
    else
        echo "Using existing source. Skipping clone."
    fi
fi

if [ ! -d "$BOINC_DIR" ]; then
    echo "Cloning BOINC repository..."
    git clone https://github.com/BOINC/boinc.git "$BOINC_DIR"
fi

cd "$BOINC_DIR"

echo ""
echo "Running autosetup..."
./_autosetup

echo ""
echo "Configuring BOINC..."
./configure --disable-client --disable-manager

echo ""
echo "Building BOINC (this takes 10-15 minutes)..."
echo "Using -j1 to avoid race conditions..."
make -j1

echo ""
echo "Building BOINC client separately..."
cd lib
make libboinc_crypt.la
cd ../client
make -j1 boinc_client

echo ""
echo "======================================================================"
echo "BOINC build complete!"
echo "======================================================================"
echo ""
echo "Binaries location:"
echo "  Server tools: $BOINC_DIR/tools/"
echo "  Client: $BOINC_DIR/client/boinc_client"
echo ""
echo "Next: Run ./setup_server.sh"
echo ""
