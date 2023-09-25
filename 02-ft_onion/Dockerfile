FROM        debian:latest

RUN         apt-get update -y                           \
            && apt-get install -y nginx                 \
            && apt-get install -y openssh-server        \
            && apt-get install -y zstd                  \
            && apt-get install -y tor                   \
            && apt-get upgrade -y                       \
            && rm -f /etc/nginx/sites-enabled/*         \
            && rm -f /var/www/html/*

COPY        ./website/*         /var/www/html/
COPY        ./nginx.conf        /etc/nginx/nginx.conf
COPY        ./sshd_config       /etc/ssh/sshd_config
COPY        ./torrc             /etc/tor/torrc
COPY        ./entrypoint.sh     /entrypoint.sh

RUN         chmod u=rx /entrypoint.sh

ENTRYPOINT  ["/entrypoint.sh"]
CMD         ["nginx", "-g", "daemon off;"]
