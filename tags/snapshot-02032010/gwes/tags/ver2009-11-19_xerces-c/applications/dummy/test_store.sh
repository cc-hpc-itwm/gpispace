#!/bin/bash
######################################################################
# $Id$
#
# Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
# for its Fraunhofer Institute for Computer Architecture and Software
# Technology (FIRST), Berlin, Germany 
# All rights reserved. 
#
######################################################################
#
# @author Andreas Hoheisel
# @version $Id$
#
# exit code 0: OK
# exit code 1: WARNING
# exit code 2: CRITICAL
# exit code 3: UNKNOWN
#
######################################################################
WRAPPERDIR=`dirname $0`
SCRIPT=$WRAPPERDIR/store.sh

# /home/poseidon/hoheisel/Projekte/SPA/svn-spa/gwes/trunk/applications/dummy/store.sh -OffsetClass  -OffsetVolume /tmp/.gwes/test_2_5/Volume.test_2_5.dat -OutputGather /tmp/.gwes/test_1_3/OutputGather.test_1_3.dat -simulation /tmp/.gwes/test_1_20/simulation.test_1_20.dat
######################
OUTPUT_GATHER=$WRAPPERDIR/test/createOutputGather_outputgather.dat
SIMULATION=$WRAPPERDIR/test/store_simulation.dat
LOG=$WRAPPERDIR/test/store.stdout
ERR=$WRAPPERDIR/test/store.stderr
######################
PARAMI=1
while [ $# -gt 0 ]
do
  case $1 in
    -i)
      INDEX=$2
      echo "# i=$INDEX"
      shift 2
      ;;
    *)
      param[${PARAMI}]="$1"
      let "PARAMI += 1"
      shift
      ;;
  esac
done
if [ -z "$INDEX" ]; then
  INDEX=0
fi
OFFSET_CLASS=$WRAPPERDIR/test/getNextOffsetClass_offsetclass_$INDEX.dat
OFFSET_VOLUME=$WRAPPERDIR/test/createVolume_volume_$INDEX.dat
#######################
rm -f $SIMULATION $LOG $ERR
echo "$SCRIPT -OffsetClass $OFFSET_CLASS -OffsetVolume $OFFSET_VOLUME -OutputGather $OUTPUT_GATHER -simulation $SIMULATION"
$SCRIPT -OffsetClass $OFFSET_CLASS -OffsetVolume $OFFSET_VOLUME -OutputGather $OUTPUT_GATHER -simulation $SIMULATION > $LOG 2> $ERR
# evaluate exit code
EXITCODE=$?
case $EXITCODE in
    0)
      echo "OK"
      exit 0
      ;;
    1)
      echo "CRITICAL"
      exit 2
      ;;
    2)
      echo "CRITICAL"
      exit 2
      ;;
    3)
      echo "CRITICAL"
      exit 2
      ;;
    4)
      echo "CRITICAL"
      exit 2
      ;;
    5)
      echo "CRITICAL"
      exit 2
      ;;
    *)
      echo "UNKNOWN"
      exit 3
      ;;
esac
