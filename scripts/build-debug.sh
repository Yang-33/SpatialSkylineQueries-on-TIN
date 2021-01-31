#!/bin/sh

DIR=out/debug
CMAKE=cmake

#rm -rf $DIR
mkdir -p ${DIR}

JOBS=$(nproc)
${CMAKE} -B${DIR} -H. -DCMAKE_BUILD_TYPE=Debug || exit 1
${CMAKE} --build ${DIR} -- "VERBOSE=1 -j ${JOBS}" || exit 1
