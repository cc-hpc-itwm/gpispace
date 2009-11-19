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
SCRIPT=$WRAPPERDIR/addTraceContribution.sh

# addTraceContribution.sh -OffsetVolume /tmp/.gwes/test_1_1/Volume.test_1_1.dat -Trace /tmp/.gwes/test_1_2/Trace.test_1_2.dat -simulation /tmp/.gwes/test_1_9/simulation.test_1_9.dat
######################
SIMULATION=$WRAPPERDIR/test/addTraceContribution_simulation.dat
LOG=$WRAPPERDIR/test/addTraceContribution.stdout
ERR=$WRAPPERDIR/test/addTraceContribution.stderr
######################
PARAMI=1
while [ $# -gt 0 ]
do
  case $1 in
    -i)
      INDEX_I=$2
      echo "# i=$INDEX_I"
      shift 2
      ;;
    -j)
      INDEX_J=$2
      echo "# j=$INDEX_J"
      shift 2
      ;;
    *)
      param[${PARAMI}]="$1"
      let "PARAMI += 1"
      shift
      ;;
  esac
done
if [ -z "$INDEX_I" ]; then
  INDEX_I=0
fi
if [ -z "$INDEX_J" ]; then
  INDEX_J=0
fi
OFFSET_VOLUME=$WRAPPERDIR/test/createVolume_volume_${INDEX_I}.dat
TRACE=$WRAPPERDIR/test/getNextTrace_trace_${INDEX_I}_${INDEX_J}.dat
######################
rm -f $SIMULATION $LOG $ERR
echo "$SCRIPT -OffsetVolume $OFFSET_VOLUME -Trace $TRACE -simulation $SIMULATION"
$SCRIPT -OffsetVolume $OFFSET_VOLUME -Trace $TRACE -simulation $SIMULATION > $LOG 2> $ERR
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
