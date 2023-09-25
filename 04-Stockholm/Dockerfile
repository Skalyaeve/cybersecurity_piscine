FROM            debian:latest

RUN             apt-get update -y                       \
                && apt-get install -y libssl-dev        \
                && mkdir -p /root/infection/sub1        \
                && mkdir -p /root/infection/sub2        \
                && touch /root/infection/test1.txt      \
                && touch /root/infection/sub1/test2.txt \
                && touch /root/infection/sub2/test3.txt

COPY            ./Stockholm     /usr/bin/Stockholm

WORKDIR         /root

CMD             ["sleep", "infinity"]