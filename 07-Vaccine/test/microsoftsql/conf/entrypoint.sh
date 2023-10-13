#!/bin/bash
/opt/mssql/bin/sqlservr > /dev/null 2>&1 & 

while ! /opt/mssql-tools/bin/sqlcmd -S localhost -U SA -P "<MyStr0ngP@ssword!>" -Q "SELECT 1;" > /dev/null 2>&1; do
    sleep 1
    echo "Waiting for Microsoft SQL server to start..."
done

/opt/mssql-tools/bin/sqlcmd -S localhost -U SA -P "<MyStr0ngP@ssword!>" -i /init.sql > /dev/null 2>&1
exec $@
