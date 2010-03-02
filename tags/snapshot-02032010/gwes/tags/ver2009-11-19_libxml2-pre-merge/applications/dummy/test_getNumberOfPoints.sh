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
SCRIPT=$WRAPPERDIR/getNumberOfPoints.sh

#./getNumberOfPoints.sh -GatherParameters 1000x800x400 -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -simulation /tmp/.gwes/test_1_20/simulation.test_1_20.dat
######################
GATHER_PARAMETERS="1000x800x400"
NUMBER_POINTS=$WRAPPERDIR/test/getNumberOfPoints_numberpoints.dat
SIMULATION=$WRAPPERDIR/test/getNumberOfPoints_simulation.dat
LOG=$WRAPPERDIR/test/getNumberOfPoints.stdout
ERR=$WRAPPERDIR/test/getNumberOfPoints.stderr
######################
rm -f $NUMBER_POINTS $SIMULATION $LOG $ERR
echo "$SCRIPT -GatherParameters $GATHER_PARAMETERS -NumberPoints $NUMBER_POINTS -simulation $SIMULATION"
$SCRIPT -GatherParameters $GATHER_PARAMETERS -NumberPoints $NUMBER_POINTS -simulation $SIMULATION > $LOG 2> $ERR
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
