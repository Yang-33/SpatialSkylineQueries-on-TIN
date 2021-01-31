#!/bin/bash
# Time
timecolumn="$1"
echo -en "${timecolumn}\t"
grep -Po "[0-9]* (?=milli)" log/glog/tin-skyline.INFO | xargs echo | tr ' ' '\t'
