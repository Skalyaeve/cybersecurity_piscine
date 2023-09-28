FROM            debian:latest

RUN             apt-get update -y                       \
                && apt-get install iproute2 -y          \
                && apt-get install net-tools -y         \
                && apt-get install vsftpd -y            \
                && useradd -m ftp_client                \
                && mkdir -p /var/run/vsftpd/empty       \
                && echo "ftp_client:that_was_ez" | chpasswd

COPY           vsftpd.conf /etc/vsftpd.conf

EXPOSE          21

CMD             ["vsftpd", "/etc/vsftpd.conf"]