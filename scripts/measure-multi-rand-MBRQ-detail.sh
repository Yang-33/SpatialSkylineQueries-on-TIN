#!/bin/bash
set -eu
# Change this
input=(1 2 3 4)
arg="-QMBRPercentage"

seedcount=$1

# Compile
./scripts/build-release.sh

mkdir -p log/summary
detailArgs=("-useBB" "-useTSILSI -useSibori" "-useIneq" "-useNewLB -useIneq" "-usetinfilter")

for ((i = 0; i < ${#detailArgs[@]}; i++)); do
  detailArg="${detailArgs[$i]}"
  title="3QMBRDetail${detailArg//[[:blank:]]/}"
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
