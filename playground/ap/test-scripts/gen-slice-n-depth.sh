#!/bin/sh
if [ $# != 2 ]; then
  echo "usage: $0 number_of_depth_levels number_of_slices"
  exit 1
fi 
nd=$1
ns=$2

for s in `seq 0 $(( $ns - 1 ))`
do
  echo "# slice $s"
  for d in `seq 0 $(( $nd - 1 ))`
  do
	slice_and_depth=$(( $nd * $s + $d ))
	echo "$slice_and_depth"
  done
done
