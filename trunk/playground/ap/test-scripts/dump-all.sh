#!/bin/sh
hdl=2
iter=0
temp=23364
if [ -n "$1" ]; then
  if [ "$1" = "-h" ]; then
	echo "usage: $0 [iter=0 [handle=2 [temp=23364]]]"
	exit 0
  fi
  iter=$1
  shift
fi

if [ -n "$1" ]; then
  hdl=$1
  shift
fi

if [ -n "$1" ]; then
  temp=$1
  shift
fi

nre-pcc -f debug.dump_global_memory -i handle=${hdl} -i offset=140      -i size=16000000                   -i file=/scratch/petry/input.${iter}  -o error_code
nre-pcc -f debug.dump_global_memory -i handle=${hdl} -i offset=16000140 -i size=$(( 24000140 - 16000140 )) -i file=/scratch/petry/velo.${iter}   -o error_code
nre-pcc -f debug.dump_global_memory -i handle=${hdl} -i offset=24000140 -i size=$(( 32000140 - 24000140 )) -i file=/scratch/petry/output.${iter} -o error_code
nre-pcc -f debug.dump_global_memory -i handle=${hdl} -i offset=32000140 -i size=$(( 32000940 - 32000140 )) -i file=/scratch/petry/vmin.${iter}   -o error_code
nre-pcc -f debug.dump_global_memory -i handle=${hdl} -i offset=32000940 -i size=$(( 800 ))                 -i file=/scratch/petry/vmax.${iter}   -o error_code
nre-pcc -f debug.dump_global_memory -i handle=6 -i offset=0 -i size=$(( 2 * 23364 )) -i file=/scratch/petry/temp.${iter} -o error_code
