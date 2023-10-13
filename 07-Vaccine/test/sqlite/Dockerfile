FROM alpine:latest

RUN apk update --no-cache && \
        apk add --no-cache \
        sqlite \
        python3 \
        py3-pip

RUN pip3 install Flask

COPY conf/backend.py /backend.py
COPY conf/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8484

ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/backend.py"]
