FROM    ubuntu:latest

RUN     echo "Etc/UTC" > /etc/timezone && \
        DEBIAN_FRONTEND=noninteractive apt-get update && \
        DEBIAN_FRONTEND=noninteractive apt-get install -y -q --no-install-recommends \
        postgresql postgresql-contrib nodejs npm

COPY    conf/backend.js /backend.js
COPY    conf/package.json /package.json
RUN     npm install

COPY    conf/entrypoint.sh /entrypoint.sh
RUN     chmod +x /entrypoint.sh

EXPOSE  8383

ENTRYPOINT ["/entrypoint.sh"]
CMD ["node", "/backend.js"]
