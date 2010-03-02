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
# Script for dummy application getNumberOfPoints.
#
# @author Andreas Hoheisel
# @version $Id$
#
# ./getNumberOfPoints.sh -GatherParameters 1000x800x400 -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -simulation /tmp/.gwes/test_1_20/simulation.test_1_20.dat
######################################################################
set -e
WRAPPERDIR=`dirname $0`

function usage {
  echo "### USAGE: getNumberOfPoints.sh -GatherParameters <XXXXxYYYYxZZZZ> -NumberPoints <NUMBER_POINTS_OUTPUT_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -GatherParameters)
      # XXXXxYYYYxZZZZ
      x=${2%%x*}
      echo "# x=$x"
      XXXXxYYYY=${2%x*}
      y=${XXXXxYYYY#*x}
      echo "# y=$y"
      z=${2##*x}
      echo "# z=$z"
      shift 2
      ;;
    -NumberPoints)
      numberpoints=$2
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

if [ -z "$x" -o -z "$y" -o -z "$z" ]; then
  echo "### ERROR: missing or wrong format of input parameter '-GatherParameters'!" 1>&2
  usage
  exit 2
fi 

if [ -z "$numberpoints" ]; then
  echo "### ERROR: missing output parameter '-NumberPoints'!" 1>&2
  usage
  exit 3
fi

####################################
value=$(($x*$y*$z))
echo "# value=$value"

echo "<numberpoints>$value</numberpoints>" > $numberpoints

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>20</cpu><io>3</io><memory><fvm>10000</fvm></memory></simulation>" > $simulation
fi

 
