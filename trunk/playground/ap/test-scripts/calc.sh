#!/bin/sh
if [ $# != 4 ]; then
  echo "usage: $0 slice_and_depth number_of_depth_levels memhandle_for_config memhandle_for_temp_output_volume"
  exit 1
fi 

nre-pcc -f calc@calc -i slice_and_depth="$1" -i number_of_depthlevels="$2" -i memhandle_for_configuration="$3" -i memhandle_for_temp_output_volume_calc="$4" -o slice_and_depth_OUT
