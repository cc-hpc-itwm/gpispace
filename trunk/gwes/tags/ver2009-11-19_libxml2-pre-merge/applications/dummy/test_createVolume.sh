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
SCRIPT=$WRAPPERDIR/createVolume.sh

# /home/poseidon/hoheisel/Projekte/SPA/svn-spa/gwes/trunk/applications/dummy/createVolume.sh -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -Volume /tmp/.gwes/test_2_26/Volume.test_2_26.dat -simulation /tmp/.gwes/test_2_26/simulation.test_2_26.dat 
######################
NUMBER_POINTS=$WRAPPERDIR/test/getNumberOfPoints_numberpoints.dat
SIMULATION=$WRAPPERDIR/test/createVolume_simulation.dat
LOG=$WRAPPERDIR/test/createVolume.stdout
ERR=$WRAPPERDIR/test/createVolume.stderr
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
VOLUME=$WRAPPERDIR/test/createVolume_volume_$INDEX.dat
#######################
rm -f $VOLUME $SIMULATION $LOG $ERR
echo "$SCRIPT -NumberPoints $NUMBER_POINTS -Volume $VOLUME -simulation $SIMULATION"
$SCRIPT -NumberPoints $NUMBER_POINTS -Volume $VOLUME -simulation $SIMULATION > $LOG 2> $ERR
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
