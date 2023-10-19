#!/bin/sh
[ -d "/var/run/mysqld" ] || mkdir /var/run/mysqld
chown mysql:mysql /var/run/mysqld

mysqld --initialize --user=mysql --datadir=/var/lib/mysql
mysqld --user=mysql &
sleep 1

while ! mysqladmin ping -h localhost --silent > /dev/null 2>&1; do
        sleep 1
done

mysql <<EOF
DELETE FROM mysql.user WHERE User='';
ALTER USER 'root'@'localhost' IDENTIFIED WITH 'mysql_native_password' BY '123';
FLUSH PRIVILEGES;

CREATE DATABASE IF NOT EXISTS test;
USE test;

CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL
);
INSERT INTO users (login, password) VALUES ('user', 'password1');
INSERT INTO users (login, password) VALUES ('admin', '<!v3rY_sTr0nG_aDmN1N_pAsSwOrD![ 1 ]>');

CREATE TABLE IF NOT EXISTS superusers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    superkey VARCHAR(255) NOT NULL
);
INSERT INTO superusers (login, password, superkey) VALUES ('superadmin', '<!!!v3rY_sTr0nG_aDmN1N_pAsSwOrD!!![ 1 ]>', '123');

CREATE DATABASE IF NOT EXISTS ab;
USE ab;

CREATE TABLE IF NOT EXISTS cd (
     ef INT AUTO_INCREMENT PRIMARY KEY,
     gh VARCHAR(255) NOT NULL
);
INSERT INTO cd (gh) VALUES ('ij');
EOF
exec $@
