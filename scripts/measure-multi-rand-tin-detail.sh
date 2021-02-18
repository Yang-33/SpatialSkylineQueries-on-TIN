#!/bin/bash
set -eu
# Change this
input=("./data/q3-0001000.off" "./data/q3-0002000.off" "./data/q3-0003000.off" "./data/q3-0004000.off")
arg="-tinpath"

seedcount=$1

# Compile
./scripts/build-release.sh

# Pre build
mkdir -p log/summary
detailArgs=("-useBB" "-useTSILSI -useSibori" "-useIneq" "-useNewLB -useIneq" "-usetinfilter")

for ((i = 0; i < ${#detailArgs[@]}; i++)); do
  detailArg="${detailArgs[$i]}"
  title="4tinpathDetail${detailArg//[[:blank:]]/}"
  Date=$(date "+%m%d-%H%M%S")
  timefile="log/summary/time-${title}-randrange${seedcount}-${Date}.txt"
  dsfile="log/summary/ds-${title}-rangdrange${seedcount}-${Date}.txt"

  echo -e "${title} (time)\tnaive\tproposal" >>${timefile}
  echo -e "${title} (ds)\tnaive\tproposal" >>${dsfile}

  for val in ${input[@]}; do
    for seed in $(seq 0 $(($seedcount - 1))); do
      ./out/release/tin-skyline -testmemods ${arg}=${val} -randseed=${seed} -qpossquare=true -forcepointupdate -reorder=2 $detailArg
      # take time : ds for naive and fast.
      ./scripts/measure/time-grep.bash "${val}" >>${timefile}
      ./scripts/measure/ds-grep.bash "${val}" >>${dsfile}
    done
  done

  # Write top file
  cp ${timefile} "log/summary/time-${title}-randrange${seedcount}.txt"
  cp ${dsfile} "log/summary/ds-${title}-rangdrange${seedcount}.txt"
done
