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
SCRIPT=$WRAPPERDIR/createOutputGather.sh

# ./createOutputGather.sh -NumberClasses 19 -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -OutputGather /tmp/.gwes/test_1_21/OutputGather.test_1_21.dat -simulation /tmp/.gwes/test_1_21/simulation.test_1_21.dat 
######################
NUMBER_CLASSES=6
NUMBER_POINTS=$WRAPPERDIR/test/getNumberOfPoints_numberpoints.dat
OUTPUT_GATHER=$WRAPPERDIR/test/createOutputGather_outputgather.dat
SIMULATION=$WRAPPERDIR/test/createOutputGather_simulation.dat
LOG=$WRAPPERDIR/test/createOutputGather.stdout
ERR=$WRAPPERDIR/test/createOutputGather.stderr
######################
rm -f $OUTPUT_GATHER $SIMULATION $LOG $ERR
echo "$SCRIPT -NumberClasses $NUMBER_CLASSES -NumberPoints $NUMBER_POINTS -OutputGather $OUTPUT_GATHER -simulation $SIMULATION"
$SCRIPT -NumberClasses $NUMBER_CLASSES -NumberPoints $NUMBER_POINTS -OutputGather $OUTPUT_GATHER -simulation $SIMULATION > $LOG 2> $ERR
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
