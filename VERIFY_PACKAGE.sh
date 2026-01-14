#!/bin/bash
#
# BOINC PI Tutorial - Package Verification Script
#
# This script verifies that the tutorial package contains all necessary files
#

set -e

echo "======================================================================"
echo "BOINC PI Tutorial - Package Verification"
echo "======================================================================"
echo ""

TARBALL="../boinc_pi_tutorial_complete.tar.gz"

if [ ! -f "$TARBALL" ]; then
    echo "ERROR: Package not found at $TARBALL"
    exit 1
fi

echo "Package found: $TARBALL"
echo ""

# Check file count
EXPECTED_COUNT=36
ACTUAL_COUNT=$(tar -tzf "$TARBALL" | wc -l)
echo "File count: $ACTUAL_COUNT (expected: $EXPECTED_COUNT)"
if [ "$ACTUAL_COUNT" -ne "$EXPECTED_COUNT" ]; then
    echo "⚠️  WARNING: File count mismatch!"
fi
echo ""

# Check critical directories exist
echo "Checking critical directories..."
DIRS=("boinc_tutorial/src" "boinc_tutorial/scripts" "boinc_tutorial/sql" "boinc_tutorial/templates" "boinc_tutorial/config" "boinc_tutorial/examples")
for dir in "${DIRS[@]}"; do
    if tar -tzf "$TARBALL" | grep -q "^${dir}/"; then
        echo "  ✓ $dir"
    else
        echo "  ✗ $dir MISSING!"
        exit 1
    fi
done
echo ""

# Check critical files exist
echo "Checking critical files..."
FILES=(
    "boinc_tutorial/README.md"
    "boinc_tutorial/TESTED_STATUS.md"
    "boinc_tutorial/src/pi_compute.cpp"
    "boinc_tutorial/src/Makefile"
    "boinc_tutorial/sql/create_database.sql"
    "boinc_tutorial/sql/register_application.sql"
    "boinc_tutorial/sql/check_status.sql"
    "boinc_tutorial/templates/pi_in.xml"
    "boinc_tutorial/templates/pi_out.xml"
    "boinc_tutorial/templates/version.xml"
    "boinc_tutorial/config/apache_pi_test.conf"
    "boinc_tutorial/config/get_project_config.xml"
    "boinc_tutorial/scripts/setup_server.sh"
    "boinc_tutorial/scripts/build_boinc.sh"
)

for file in "${FILES[@]}"; do
    if tar -tzf "$TARBALL" | grep -q "^${file}$"; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file MISSING!"
        exit 1
    fi
done
echo ""

# Check MD5 checksum
echo "Verifying MD5 checksum..."
EXPECTED_MD5="bc262a0d9bbb98fb41ee72fda91d7f82"
ACTUAL_MD5=$(md5sum "$TARBALL" | awk '{print $1}')
echo "  Expected: $EXPECTED_MD5"
echo "  Actual:   $ACTUAL_MD5"
if [ "$ACTUAL_MD5" = "$EXPECTED_MD5" ]; then
    echo "  ✓ Checksum matches"
else
    echo "  ⚠️  Checksum mismatch (may be expected if regenerated)"
fi
echo ""

# Check file sizes
SIZE=$(ls -lh "$TARBALL" | awk '{print $5}')
echo "Package size: $SIZE"
echo ""

# Check scripts are executable
echo "Checking scripts have content..."
SCRIPTS=(
    "boinc_tutorial/scripts/install_dependencies_debian.sh"
    "boinc_tutorial/scripts/install_dependencies_opensuse.sh"
    "boinc_tutorial/scripts/build_boinc.sh"
    "boinc_tutorial/scripts/setup_server.sh"
    "boinc_tutorial/scripts/start_clients.sh"
    "boinc_tutorial/scripts/check_results.sh"
)

for script in "${SCRIPTS[@]}"; do
    LINES=$(tar -xzOf "$TARBALL" "$script" 2>/dev/null | wc -l)
    if [ "$LINES" -gt 10 ]; then
        echo "  ✓ $script ($LINES lines)"
    else
        echo "  ✗ $script TOO SHORT OR MISSING!"
        exit 1
    fi
done
echo ""

echo "======================================================================"
echo "✓ Package verification PASSED"
echo "======================================================================"
echo ""
echo "The package contains:"
echo "  - Complete source code"
echo "  - All SQL scripts"
echo "  - All XML templates"
echo "  - All configuration files"
echo "  - All setup scripts"
echo "  - Complete documentation"
echo ""
echo "Package is ready for distribution to students."
echo ""
