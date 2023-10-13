FROM    mcr.microsoft.com/mssql/server:latest

ENV    ACCEPT_EULA=Y
ENV    SA_PASSWORD=<MyStr0ngP@ssword!>

USER    root
RUN     apt-get update && apt-get install -y \
        curl nodejs npm

COPY    conf/backend.js /backend.js
COPY    conf/package.json /package.json
RUN     npm install

COPY    conf/init.sql /init.sql
COPY    conf/entrypoint.sh /entrypoint.sh
RUN     chmod +x /entrypoint.sh

EXPOSE  8585

ENTRYPOINT ["/entrypoint.sh"]
CMD ["node", "/backend.js"]
