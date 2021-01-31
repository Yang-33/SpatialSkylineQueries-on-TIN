#!/bin/bash

type=$1
tinfile=$2
arg=$3

./scripts/build-${type}.sh && \
./out/${type}/tin-skyline -tinpath=${tinfile} ${arg} && \
meshlab ${tinfile}  viz.off
