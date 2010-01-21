#!/bin/sh
if [ $# != 1 ]; then
  echo "usage: $0 memhandle_for_config"
  exit 1
fi 

nre-pcc -f writeoutp.writeoutp -i memhandle_for_configuration="$1" -o seq -o output_file
