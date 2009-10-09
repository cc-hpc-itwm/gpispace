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
SCRIPT=$WRAPPERDIR/getNextOffsetClass.sh

#/getNextOffsetClass.sh -HasNext true -OffsetParameters 15.33.1. -i 0 -HasNext /tmp/.gwes/test_1_1/HasNext.test_1_1.dat -OffsetClass /tmp/.gwes/test_1_1/OffsetClass.test_1_1.dat
######################
HASNEXT_IN=true
OFFSET_PARAMETERS="15.33.1."
HASNEXT_OUT=$WRAPPERDIR/test/getNextOffsetClass_hasnext.dat
SIMULATION=$WRAPPERDIR/test/getNextOffsetClass_simulation.dat
LOG=$WRAPPERDIR/test/getNextOffsetClass.stdout
ERR=$WRAPPERDIR/test/getNextOffsetClass.stderr
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
#######################

if [ -z "$INDEX" ]; then
  INDEX=0
fi

OFFSETCLASS=$WRAPPERDIR/test/getNextOffsetClass_offsetclass_$INDEX.dat


rm -f $HASNEXT_OUT $OFFSETCLASS $SIMULATION $LOG $ERR
#echo "$SCRIPT -HasNext $HASNEXT_IN -OffsetParameters $OFFSET_PARAMETERS -i $INDEX -HasNext $HASNEXT_OUT -OffsetClass $OFFSETCLASS"
$SCRIPT -HasNext $HASNEXT_IN -OffsetParameters $OFFSET_PARAMETERS -i $INDEX -HasNext $HASNEXT_OUT -OffsetClass $OFFSETCLASS -simulation $SIMULATION > $LOG 2> $ERR
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
