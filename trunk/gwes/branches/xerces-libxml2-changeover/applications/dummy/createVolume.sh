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
# Script for dummy application createVolume
#
# @author Andreas Hoheisel
# @version $Id$
#
# /home/poseidon/hoheisel/Projekte/SPA/svn-spa/gwes/trunk/applications/dummy/createVolume.sh -NumberPoints /tmp/.gwes/test_1_20/NumberPoints.test_1_20.dat -Volume /tmp/.gwes/test_2_26/Volume.test_2_26.dat
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
    -NumberPoints)
      numberpointsfn=$2
      shift 2
      ;;
    -Volume)
      volumefn=$2
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

if [ -z "$numberpointsfn" ]; then
  echo "### ERROR: missing input parameter '-NumberPoints'!" 1>&2
  usage
  exit 2
fi

if [ ! -r "$numberpointsfn" ]; then
  echo "### ERROR: cannot read input file '$numberpointsfn'!" 1>&2
  usage
  exit 3
fi
  
if [ -z "$volumefn" ]; then
  echo "### ERROR: missing output parameter '-Volume'!" 1>&2
  exit 4
fi

####################################
# <data><numberpoints>320000000</numberpoints></data>
np1=`cat $numberpointsfn`
np2=${np1#*<numberpoints>}
numberpoints=${np2%</numberpoints>*}
echo "# numberpoints=$numberpoints"
memory=$(($numberpoints*16))
echo "# memory=$memory"

echo "<!-- Volume for $numberpoints points -->" > $volumefn
echo "<simulation><memory><fvm>$memory</fvm></memory></simulation>" >> $volumefn

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>80</cpu><io>100</io><memory edgeExpression=\"Volume\"><fvm>$memory</fvm></memory></simulation>" > $simulation
fi
