FROM            debian:latest

RUN             apt-get update -y   \
                && mkdir /infection

COPY            ./Stockholm     /.
COPY            ./entrypoint.sh /.

RUN             chmod +x /entrypoint.sh
ENTRYPOINT      ["/entrypoint.sh"]