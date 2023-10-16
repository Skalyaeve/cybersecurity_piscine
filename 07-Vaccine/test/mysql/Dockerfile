FROM    ubuntu:latest

RUN     apt-get update &&       \
        apt-get install -y      \
        mysql-server            \
        mysql-client            \
        nodejs npm

COPY    conf/backend.js /backend.js
COPY    conf/package.json /package.json
RUN     npm install

COPY    conf/entrypoint.sh /entrypoint.sh
RUN     chmod +x /entrypoint.sh

EXPOSE  8282

ENTRYPOINT ["/entrypoint.sh"]
CMD ["node", "/backend.js"]
