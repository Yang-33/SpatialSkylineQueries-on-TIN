#!/bin/bash
set -eu
# Change this
title="QMBRPercentage"
input=(0.1 0.5 1 5 10 50)
arg="-QMBRPercentage"
tinfile="./data/q3-0000500.off"

seed=$1

# Pre build
mkdir -p log/summary
timefile="log/summary/time-${title}-$(date "+%m%d-%H%M%S").txt"
dsfile="log/summary/ds-${title}-$(date "+%m%d-%H%M%S").txt"

# Compile
./scripts/build-release.sh

# Write out
echo -e "${title} (time)\tnaive\tproposal" >>${timefile}
echo -e "${title} (ds)\tnaive\tproposal" >>${dsfile}

for val in ${input[@]}; do # !!
  ./out/release/tin-skyline -tinpath=${tinfile} -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate
  ./scripts/measure/time-grep.bash "${val}" >>${timefile}
  ./scripts/measure/ds-grep.bash "${val}" >>${dsfile}
done

# Transpose
cat ${timefile} | datamash transpose >${timefile%.txt}-tr.txt
cat ${dsfile} | datamash transpose >${dsfile%.txt}-tr.txt

# Write top file
cp ${timefile%.txt}-tr.txt "log/summary/time-${title}-tr.txt"
cp ${dsfile%.txt}-tr.txt "log/summary/ds-${title}-tr.txt"
