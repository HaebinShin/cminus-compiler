FROM ubuntu:xenial
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y build-essential gcc flex bison