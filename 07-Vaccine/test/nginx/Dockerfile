FROM    alpine:latest

RUN     apk update --no-cache   \
        && apk add --no-cache nginx

COPY    conf/nginx.conf         /etc/nginx/nginx.conf
COPY    conf/default.conf       /etc/nginx/http.d/default.conf
COPY    www                     /srv/http/sqltest

CMD     ["nginx", "-g", "daemon off;"]