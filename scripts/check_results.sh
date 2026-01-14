#!/bin/bash
# Check computation results

DB_USER="boincadm"
DB_PASS="boinc123"
DB_NAME="pi_test"

echo "======================================================================"
echo "BOINC PI Computation Results"
echo "======================================================================"
echo ""

mysql -u $DB_USER -p"$DB_PASS" $DB_NAME << 'EOF'
SELECT 
    w.name as work_unit,
    r.name as result_name,
    r.server_state,
    r.outcome,
    DATE_FORMAT(FROM_UNIXTIME(r.received_time), '%Y-%m-%d %H:%i:%s') as completed
FROM workunit w
JOIN result r ON w.id = r.workunitid
WHERE r.server_state >= 4
ORDER BY w.id, r.id;
EOF

echo ""
echo "Checking uploaded result files..."
find ~/boinc_test/projects/upload -type f -name "*.txt" -o -name "*_0" | while read file; do
    echo ""
    echo "=== $(basename $file) ==="
    grep "Estimated value of PI" "$file" 2>/dev/null || echo "No PI value found"
done

echo ""
