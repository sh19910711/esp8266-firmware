FROM ubuntu:16.04
MAINTAINER Seiya Nuta <nuta@seiya.me>

RUN apt-get update && apt-get install -qy build-essential curl git
ADD . /esp8266-firmware
WORKDIR /esp8266-firmware
RUN curl http://arduino.esp8266.com/linux64-xtensa-lx106-elf-gb404fb9.tar.gz | tar zxf -

RUN make -C esptool-ck-0.4.9 clean all
ENV PATH /esp8266-firmware/xtensa-lx106-elf/bin:$PATH
RUN make
