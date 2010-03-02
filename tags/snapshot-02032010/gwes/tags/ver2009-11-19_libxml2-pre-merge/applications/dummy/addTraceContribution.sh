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
# Script for dummy application addTraceContribution
#
# @author Andreas Hoheisel
# @version $Id$
#
# addTraceContribution.sh -OffsetVolume /tmp/.gwes/test_1_1/Volume.test_1_1.dat -Trace /tmp/.gwes/test_1_2/Trace.test_1_2.dat -simulation /tmp/.gwes/test_1_9/simulation.test_1_9.dat
######################################################################
set -e
WRAPPERDIR=`dirname $0`

########### configuration #############
cpu=8000
io=100
# memory per offset class
memory=10
# edge expression of output data related to memory consumption
#edgeExpression=Trace
#######################################

function usage {
  echo "### USAGE: addTraceContribution.sh -OffsetVolume <OFFSET_VOLUME_FILE> -Trace <TRACE_INPUT_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -OffsetVolume)
      offsetvolumefn=$2
      shift 2
      ;;
    -Trace)
      tracefn=$2
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

if [ -z "$offsetvolumefn" ]; then
  echo "### ERROR: missing input parameter '-OffsetVolume'!" 1>&2
  usage
  exit 2
fi

if [ ! -r "$offsetvolumefn" ]; then
  echo "### ERROR: cannot read OffsetVolume file '$offsetvolumefn'!" 1>&2
  usage
  exit 3
fi

if [ -z "$tracefn" ]; then
  echo "### ERROR: missing output parameter '-Trace'!" 1>&2
  exit 4
fi

if [ ! -r "$tracefn" ]; then
  echo "### ERROR: cannot read Trace file '$tracefn'!" 1>&2
  usage
  exit 5
fi


####################################
echo "<trace>" >> $offsetvolumefn
echo "  `cat $tracefn | grep offsetclass`" >> $offsetvolumefn
echo "  `cat $tracefn | grep traceindex`" >> $offsetvolumefn
echo "</trace>" >> $offsetvolumefn

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>$cpu<cpu><io>$io</io><memory></memory></simulation>" > $simulation
fi
