#!/bin/sh
if [ $# != 4 ]; then
  echo "usage: $0 memhandle_for_config number_of_depth_levels slice_and_depth memhandle_for_temp_outputvolume"
  exit 1
fi 

nre-pcc -f update.update -i memhandle_for_configuration="$1" -i number_of_depthlevels="$2" -i slice_and_depth="$3" -i memhandle_for_temp_outputvolume="$4" -o memhandle_for_temp_outputvolume_OUT -o slice_and_depth_OUT
