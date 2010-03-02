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
SCRIPT=$WRAPPERDIR/getNextTrace.sh

# getNextTrace.sh -OffsetClass ./test/getNextOffsetClass_offsetclass.dat -hasNext true -i 0 -Trace /tmp/.gwes/test_1_2/Trace.test_1_2.dat -hasNext /tmp/.gwes/test_1_2/hasNext.test_1_2.dat -simulation /tmp/.gwes/test_1_2/simulation.test_1_2.dat
######################
HASNEXT_IN=true
HASNEXT_OUT=$WRAPPERDIR/test/getNextTrace_hasnext.dat
SIMULATION=$WRAPPERDIR/test/getNextTrace_simulation.dat
LOG=$WRAPPERDIR/test/getNextTrace.stdout
ERR=$WRAPPERDIR/test/getNextTrace.stderr
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
OFFSET_CLASS=$WRAPPERDIR/test/getNextOffsetClass_offsetclass_${INDEX_I}.dat
TRACE=$WRAPPERDIR/test/getNextTrace_trace_${INDEX_I}_${INDEX_J}.dat
#######################

rm -f $TRACE $HASNEXT_OUT $SIMULATION $LOG $ERR
echo "$SCRIPT -OffsetClass $OFFSET_CLASS -hasNext $HASNEXT_IN -i $INDEX -Trace $TRACE -hasNext $HASNEXT_OUT -simulation $SIMULATION"
$SCRIPT -OffsetClass $OFFSET_CLASS -hasNext $HASNEXT_IN -i $INDEX_J -Trace $TRACE -hasNext $HASNEXT_OUT -simulation $SIMULATION > $LOG 2> $ERR
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
