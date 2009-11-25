#!/bin/sh
if [ $# != 4 ]; then
  echo "usage: $0 memhandle_for_config memhandle_for_temp_outputvolume"
  exit 1
fi 

nre-pcc -f update@update -i memhandle_for_configuration="$1" -i memhandle_for_temp_outputvolume="$2" -o seq
