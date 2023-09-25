FROM            debian:latest

RUN             apt-get update -y                       \
                && apt-get install -y openssl           \
                && mkdir /var/run/IronDome              \
                && mkdir -p /root/infection/sub1        \
                && mkdir -p /root/infection/sub2        \
                && touch /root/infection/test1.txt      \
                && touch /root/infection/sub1/test2.txt \
                && touch /root/infection/sub2/test3.txt

COPY            ./IronDome              /usr/bin/IronDome
COPY            ./data/*                /var/run/IronDome/
COPY            ./openssl_cmd.sh        /root/openssl_cmd.sh

RUN            chmod +x /root/openssl_cmd.sh

WORKDIR         /root

CMD             ["sleep", "infinity"]