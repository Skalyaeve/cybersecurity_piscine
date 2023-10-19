#!/bin/sh
service postgresql start

until su - postgres -c "psql -c '\l'" > /dev/null 2>&1; do
  echo "Postgres is unavailable - sleeping"
  sleep 1
done

echo "Postgres is up - executing commands"
su - postgres -c "psql -c \"ALTER USER postgres WITH PASSWORD '123';\""
su - postgres -c "psql -c \"SELECT 1 FROM pg_database WHERE datname = 'test'\"" | grep -q 1 || su - postgres -c "psql -c \"CREATE DATABASE test\""
su - postgres -c "psql -d test" << EOF
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL
);
INSERT INTO users (login, password) VALUES ('user', 'password2');
INSERT INTO users (login, password) VALUES ('admin', '<!v3rY_sTr0nG_aDmN1N_pAsSwOrD![ 2 ]>');

CREATE TABLE IF NOT EXISTS superusers (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    superkey VARCHAR(255) NOT NULL
);
INSERT INTO superusers (login, password, superkey) VALUES ('superadmin', '<!!!v3rY_sTr0nG_aDmN1N_pAsSwOrD!!![ 1 ]>', '123');
EOF

su - postgres -c "psql -c \"CREATE DATABASE ab\""
su - postgres -c "psql -d ab" << EOF
CREATE TABLE IF NOT EXISTS cd (
    ef SERIAL PRIMARY KEY,
    gh VARCHAR(255) NOT NULL
);
INSERT INTO cd (gh) VALUES ('ij');
EOF

exec $@
