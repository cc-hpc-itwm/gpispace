#!/bin/sh
if [ $# != 1 ]; then
  echo "usage: $0 config-handle"
  exit 1
fi 

nre-pcc -f readvelo.readvelo -i memhandle_for_configuration="$1" -o seq
