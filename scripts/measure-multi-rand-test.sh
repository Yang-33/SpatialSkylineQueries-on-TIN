#!/bin/bash
set -eu
# Compile
./scripts/build-release.sh

seedcount=$1

# P
input=(100 200 300 400)
arg="-Psize"

for val in ${input[@]}; do
  for seed in $(seq 0 $(($seedcount - 1))); do
    ./out/release/tin-skyline -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2 #-onlyfast
  done
done

# Q
input=(5 10 15 20)
arg="-Qsize"

for val in ${input[@]}; do
  for seed in $(seq 0 $(($seedcount - 1))); do
    ./out/release/tin-skyline -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2 #-onlyfast
  done
done

# MBR Q
input=(1 2 3 4)
arg="-QMBRPercentage"

for val in ${input[@]}; do
  for seed in $(seq 0 $(($seedcount - 1))); do
    ./out/release/tin-skyline -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2 #-onlyfast
  done
done

# TIN
input=("./data/q3-0001000.off" "./data/q3-0002000.off" "./data/q3-0003000.off" "./data/q3-0004000.off")
arg="-tinpath"

for val in ${input[@]}; do
  for seed in $(seq 0 $(($seedcount - 1))); do
    ./out/release/tin-skyline -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2 #-onlyfast
  done
done
