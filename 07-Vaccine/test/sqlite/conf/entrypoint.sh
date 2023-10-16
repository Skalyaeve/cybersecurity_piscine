#!/bin/sh
sqlite3 test.db <<EOF
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT NOT NULL,
    password TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS superusers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT NOT NULL,
    password TEXT NOT NULL,
    superkey VARCHAR(255) NOT NULL
);
INSERT INTO users (login, password) VALUES ('user1', 'password3');
INSERT INTO users (login, password) VALUES ('admin', '<!v3rY_sTr0nG_aDmN1N_pAsSwOrD![ 3 ]>');
INSERT INTO superusers (login, password, superkey) VALUES ('superadmin', '<!!!v3rY_sTr0nG_aDmN1N_pAsSwOrD!!![ 1 ]>', '123');
EOF
exec $@