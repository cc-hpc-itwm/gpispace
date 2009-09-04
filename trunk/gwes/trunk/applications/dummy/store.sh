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
# Script for dummy application store
#
# @author Andreas Hoheisel
# @version $Id$
#
# /home/poseidon/hoheisel/Projekte/SPA/svn-spa/gwes/trunk/applications/dummy/store.sh -OffsetClass  -OffsetVolume /tmp/.gwes/test_2_5/Volume.test_2_5.dat -OutputGather /tmp/.gwes/test_1_3/OutputGather.test_1_3.dat -simulation /tmp/.gwes/test_1_20/simulation.test_1_20.dat
######################################################################
set -e
WRAPPERDIR=`dirname $0`

########### configuration #############
cpu=10
io=3000
# memory per offset class
memory=10
# edge expression of output data related to memory consumption
#edgeExpression=Trace
#######################################

function usage {
  echo "### USAGE: store.sh -OffsetClass <OFFSET_CLASS_INPUT_FILE> -OffsetVolume <OFFSET_VOLUME_INPUT_FILE> -OutputGather <OUTPUT_GATHER_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -OffsetClass)
      if [ -r $2 ]; then
        offsetclass="`cat $2 | grep offsetclass`"
      else
        offsetclass="<offsetclass>$2</offsetclass>"
      fi 
      shift 2
      ;;
    -OffsetVolume)
      offsetvolumefn=$2
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

if [ -z "$offsetclass" ]; then
  echo "### ERROR: missing input parameter '-OffsetClass'!" 1>&2
  usage
  exit 2
fi

if [ -z "$offsetvolumefn" ]; then
  echo "### ERROR: missing input parameter '-OffsetVolume'!" 1>&2
  usage
  exit 3
fi

if [ ! -r "$offsetvolumefn" ]; then
  echo "### ERROR: cannot read OffsetVolume file '$offsetvolumefn'!" 1>&2
  usage
  exit 4
fi

if [ -z "$outputgatherfn" ]; then
  echo "### ERROR: missing output parameter '-OutputGather'!" 1>&2
  usage
  exit 5
fi

####################################
echo "<storevolume>" >> $outputgatherfn
echo "  $offsetclass" >> $outputgatherfn
echo "  <traces>" >> $outputgatherfn
echo "  `cat $offsetvolumefn | grep -m 1 offsetclass`" >> $outputgatherfn
echo "  `cat $offsetvolumefn | grep traceindex`" >> $outputgatherfn
echo "  </traces>" >> $outputgatherfn
echo "</storevolume>" >> $outputgatherfn

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>$cpu<cpu><io>$io</io><memory></memory></simulation>" > $simulation
fi
