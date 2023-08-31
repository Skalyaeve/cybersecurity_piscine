#!/bin/sh
service ssh start
service tor start
exec "$@"
