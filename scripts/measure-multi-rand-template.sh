#!/bin/bash
set -eu
# Change this
title="QMBRPercentage"
input=(0.1 0.5 1 5 10 50)
arg="-QMBRPercentage"
tinfile="./data/q3-0000500.off"

seedcount=$1

# Compile
./scripts/build-release.sh

# Pre build
mkdir -p log/summary
# file : create files
Date=$(date "+%m%d-%H%M%S")
timefile="log/summary/time-${title}-randrange${seedcount}-${Date}.txt"
dsfile="log/summary/ds-${title}-rangdrange${seedcount}-${Date}.txt"

echo -e "${title} (time)\tnaive\tproposal" >>${timefile}
echo -e "${title} (ds)\tnaive\tproposal" >>${dsfile}

for val in ${input[@]}; do
  for seed in $(seq 0 $(($seedcount - 1))); do
    ./out/release/tin-skyline -tinpath=${tinfile} -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2
    # take time : ds for naive and fast.
    ./scripts/measure/time-grep.bash "${val}" >>${timefile}
    ./scripts/measure/ds-grep.bash "${val}" >>${dsfile}
  done
done

# Write top file
cp ${timefile} "log/summary/time-${title}-randrange${seedcount}.txt"
cp ${dsfile} "log/summary/ds-${title}-rangdrange${seedcount}.txt"
