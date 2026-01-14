-- BOINC PI Tutorial - Database Creation Script
--
-- Creates the MySQL/MariaDB database and user for BOINC server
--
-- Usage:
--   mysql -u root -p < create_database.sql
--
-- Note: You will need MySQL root password

-- Create database
CREATE DATABASE IF NOT EXISTS pi_test;

-- Create BOINC admin user
CREATE USER IF NOT EXISTS 'boincadm'@'localhost' IDENTIFIED BY 'boinc123';

-- Grant all privileges on the project database
GRANT ALL PRIVILEGES ON pi_test.* TO 'boincadm'@'localhost';

-- Apply changes
FLUSH PRIVILEGES;

-- Show databases
SHOW DATABASES;
SELECT User, Host FROM mysql.user WHERE User = 'boincadm';
