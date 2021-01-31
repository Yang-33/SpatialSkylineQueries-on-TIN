#!/bin/bash
# Ds
dscolumn="$1"
echo -en "${dscolumn}\t"
grep -Po "(?<=ds := )[0-9]*" log/glog/tin-skyline.INFO | xargs echo | tr ' ' '\t'
