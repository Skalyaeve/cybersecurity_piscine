#!/bin/sh
sqlite3 test.db <<EOF
DROP TABLE IF EXISTS users;
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT NOT NULL,
    password TEXT NOT NULL
);
INSERT INTO users (login, password) VALUES ('user1', 'password3');
INSERT INTO users (login, password) VALUES ('admin', '<!v3rY_sTr0nG_aDmN1N_pAsSwOrD![ 3 ]>');

DROP TABLE IF EXISTS superusers;
CREATE TABLE IF NOT EXISTS superusers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT NOT NULL,
    password TEXT NOT NULL,
    superkey TEXT NOT NULL
);
INSERT INTO superusers (login, password, superkey) VALUES ('superadmin', '<!!!v3rY_sTr0nG_aDmN1N_pAsSwOrD!!![ 1 ]>', '123');

DROP TABLE IF EXISTS cd;
CREATE TABLE IF NOT EXISTS cd (
    ef INTEGER PRIMARY KEY AUTOINCREMENT,
    gh TEXT NOT NULL
);
INSERT INTO cd (gh) VALUES ('ij');
EOF
exec $@