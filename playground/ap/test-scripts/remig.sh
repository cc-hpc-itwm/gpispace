#!/bin/sh
if [ $# != 4 ]; then
  echo "usage: $0 memhandle_for_config number_of_depth_levels number_of_slices memhandle_for_temp_outputvolume"
  exit 1
fi 
ch=$1
nd=$2
ns=$3
mh=$4

for snd in $( ./gen-slice-n-depth.sh $nd $ns ); do
  ./calc.sh   $ch $nd $snd $mh
  ./update.sh $ch $nd $snd $mh
done
