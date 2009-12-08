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
# Script for dummy application getNextOffsetClass.
#
# @author Andreas Hoheisel
# @version $Id$
#
# ./getNextOffsetClass.sh -HasNext true -OffsetParameters 15.33.1. -i 0 -HasNext /tmp/.gwes/test_1_1/HasNext.test_1_1.dat -OffsetClass /tmp/.gwes/test_1_1/OffsetClass.test_1_1.dat
######################################################################
set -e
WRAPPERDIR=`dirname $0`

########### configuration #############
cpu=350
io=10
# memory per offset class
memory=8000
# edge expression of output data related to memory consumption
edgeExpression=OffsetClass
#######################################

function usage {
  echo "### USAGE: getNextOffsetClass.sh -HasNext <true|false> -OffsetParameters <MIN.MAX.STEP.> -i <INDEX> -HasNext <HASNEXT_OUTPUT_FILE> -OffsetClass <OFFSET_CLASS_OUTPUT_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -HasNext)
      hasnext=$2
      shift 2
      ;;
    -OffsetParameters)
      # MIN.MAX.STEP.
      MIN_MAX_STEP=${2%.}
      min=${MIN_MAX_STEP%%.*}
      echo "# min=$min"
      MIN_MAX=${MIN_MAX_STEP%.*}
      max=${MIN_MAX#*.}
      echo "# max=$max"
      step=${MIN_MAX_STEP##*.}
      echo "# step=$step"
      shift 2
      ;;
    -i)
      index=$2
      echo "# i=$index"
      shift 2
      ;;
    -OffsetClass)
      offsetclass=$2
      shift 2
      ;;
    -simulation)
      simulation=$2
      shift 2
      ;;
    *)
      param[${INDEX}]="$1"
      let "INDEX += 1"
      shift
      ;;
  esac
done

if [ -z "$hasnext" ]; then
  echo "### ERROR: missing input/output parameter '-HasNext'!" 1>&2
  usage
  exit 2
fi 

if [ -z "$min" -o -z "$max" -o -z "$step" ]; then
  echo "### ERROR: missing or wrong format of input parameter '-OffsetParameters'!" 1>&2
  usage
  exit 3
fi 

if [ -z "$index" ]; then
  echo "### ERROR: missing input parameter '-i'!" 1>&2
  usage
  exit 4
fi

if [ -z "$offsetclass" ]; then
  echo "### ERROR: missing output parameter '-OffsetClass'!" 1>&2
  usage
  exit 4
fi

####################################
value=$(($min+$step*$index))
echo "# value=$value"
echo "# memory=$memory"

if [ $value -ge $min -a $value -le $max ]; then
  echo "<offsetclass>$value</offsetclass>" > $offsetclass
#  echo "<simulation><memory><fvm>$memory</fvm></memory></simulation>" >> $offsetclass
else
  echo "ERROR: value not within range [min,max]!" 1>&2
  usage
  exit 5
fi

if [ $(($value + $step)) -gt $max ]; then
  echo "<hasnext>false</hasnext>" > $hasnext
else 
  echo "<hasnext>true</hasnext>" > $hasnext
fi

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>$cpu<cpu><io>$io</io><memory><fvm edgeExpression=\"$edgeExpression\">$memory</fvm></memory></simulation>" > $simulation
fi
