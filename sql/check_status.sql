-- BOINC PI Tutorial - Status Check Queries
--
-- Useful SQL queries to check server status
--
-- Usage:
--   mysql -u boincadm -pboinc123 pi_test < check_status.sql
--   or run individual queries interactively

-- Show all work units
SELECT id, name, create_time, need_validate, canonical_resultid
FROM workunit
ORDER BY create_time DESC;

-- Show all results
SELECT id, name, workunitid, server_state, outcome, validate_state, create_time
FROM result
ORDER BY create_time DESC;

-- Show result details with output
SELECT r.id, r.name, r.server_state, r.outcome, r.validate_state, r.cpu_time,
       wu.name as workunit_name, h.domain_name as host
FROM result r
JOIN workunit wu ON r.workunitid = wu.id
LEFT JOIN host h ON r.hostid = h.id
ORDER BY r.create_time DESC;

-- Count work units by status
SELECT
    COUNT(*) as total_workunits,
    SUM(CASE WHEN canonical_resultid > 0 THEN 1 ELSE 0 END) as completed,
    SUM(CASE WHEN need_validate > 0 THEN 1 ELSE 0 END) as need_validate
FROM workunit;

-- Count results by state
SELECT
    server_state,
    COUNT(*) as count,
    CASE server_state
        WHEN 1 THEN 'INACTIVE'
        WHEN 2 THEN 'UNSENT'
        WHEN 4 THEN 'IN_PROGRESS'
        WHEN 5 THEN 'OVER'
        ELSE 'UNKNOWN'
    END as state_name
FROM result
GROUP BY server_state;

-- Show hosts (clients)
SELECT id, domain_name, create_time, rpc_time, total_credit
FROM host
ORDER BY create_time DESC;
