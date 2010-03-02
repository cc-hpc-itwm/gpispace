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
# Script for dummy application createOutputGather
#
# @author Andreas Hoheisel
# @version $Id$
#
# ./createOutputGather.sh -NumberClasses 19 -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -OutputGather /tmp/.gwes/test_1_21/OutputGather.test_1_21.dat -simulation /tmp/.gwes/test_1_21/simulation.test_1_21.dat 
######################################################################
set -e
WRAPPERDIR=`dirname $0`

function usage {
  echo "### USAGE: createOutputGather.sh -NumberClasses <NUMBER_CLASSES> -NumberPoints <NUMBER_POINTS_INPUT_FILE> -OutputGather <OUTPUT_GATHER_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -NumberClasses)
      numberclasses=$2
      echo "# numberclasses=$numberclasses"
      shift 2
      ;;
    -NumberPoints)
      numberpointsfn=$2
      shift 2
      ;;
    -OutputGather)
      outputgatherfn=$2
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

if [ -z "$numberclasses" ]; then
  echo "### ERROR: missing input parameter '-NumberClasses'!" 1>&2
  usage
  exit 2
fi 

if [ -z "$numberpointsfn" ]; then
  echo "### ERROR: missing input parameter '-NumberPoints'!" 1>&2
  usage
  exit 3
fi

if [ ! -r "$numberpointsfn" ]; then
  echo "### ERROR: cannot read input file '$numberpointsfn'!" 1>&2
  usage
  exit 4
fi
  
####################################
# <data><numberpoints>320000000</numberpoints></data>
np1=`cat $numberpointsfn`
np2=${np1#*<numberpoints>}
numberpoints=${np2%</numberpoints>*}
echo "# numberpoints=$numberpoints"
memory=$(($numberclasses*$numberpoints*16))
echo "# memory=$memory"

echo "<!-- OutputGather for $numberclasses classes and $numberpoints points -->" > $outputgatherfn
echo "<simulation><memory><fvm>$memory</fvm></memory></simulation>" >> $outputgatherfn

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>100</cpu><io>7000</io><memory edgeExpression=\"OutputGather\"><fvm>$memory</fvm></memory></simulation>" > $simulation
fi

