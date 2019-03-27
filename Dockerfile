FROM ubuntu:xenial
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential gcc flex
RUN mkdir /work_root
WORKDIR /work_root
VOLUME /work_root/build

COPY . ./
CMD make