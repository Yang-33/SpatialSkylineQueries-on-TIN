FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY host.docker.internal:0.0

RUN apt update && \
  apt install -y cmake clang-format libgflags-dev googletest libgoogle-glog-dev build-essential && \
  cd /usr/src/googletest/ && cmake . && make && make install
RUN useradd -m vscode && echo 'vscode:vscode' | chpasswd && usermod -aG sudo vscode && chsh -s /bin/bash vscode
RUN apt install -y git curl python wget unzip sudo gdb libcgal-dev libcgal-demo meshlab libeigen3-dev
RUN apt install -y tzdata && \
  ln -sf /usr/share/zoneinfo/Asia/Tokyo /etc/localtime
