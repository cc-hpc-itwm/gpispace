#!/bin/sh
if [ $# != 4 ]; then
  echo "usage: $0 memhandle_for_config number_of_depth_levels number_of_slices memhandle_for_temp_outputvolume"
  exit 1
fi 
ch=$1
nd=$2
ns=$3
mh=$4

for d in `seq 0 $(( $nd - 1 ))`
do
  for s in `seq 0 $(( $ns - 1 ))`
  do
	slice_and_depth=$(( $nd * $s + $d ))
	./calc.sh   $ch $nd $slice_and_depth $mh
	./update.sh $ch $nd $slice_and_depth $mh
  done
done
