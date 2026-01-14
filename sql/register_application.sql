-- BOINC PI Tutorial - Application Registration
--
-- Registers the PI computation application and platform in BOINC database
--
-- Usage:
--   mysql -u boincadm -pboinc123 pi_test < register_application.sql
--
-- Note: Run this AFTER make_project creates the database schema

-- Register the PI computation application
INSERT IGNORE INTO app (name, user_friendly_name, create_time)
VALUES ('pi_compute', 'PI Computation', UNIX_TIMESTAMP());

-- Register the Linux x86-64 platform
INSERT IGNORE INTO platform (name, user_friendly_name, deprecated, create_time)
VALUES ('x86_64-pc-linux-gnu', 'Linux x86-64', 0, UNIX_TIMESTAMP());

-- Show registered applications and platforms
SELECT * FROM app;
SELECT * FROM platform;
