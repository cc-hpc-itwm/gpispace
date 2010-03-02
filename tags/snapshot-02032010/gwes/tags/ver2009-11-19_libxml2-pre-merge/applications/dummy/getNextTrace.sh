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
# Script for dummy application getNextTrace
#
# @author Andreas Hoheisel
# @version $Id$
#
# getNextTrace.sh -OffsetClass ./test/getNextOffsetClass_offsetclass.dat -hasNext true -i 0 -Trace /tmp/.gwes/test_1_2/Trace.test_1_2.dat -hasNext /tmp/.gwes/test_1_2/hasNext.test_1_2.dat -simulation /tmp/.gwes/test_1_2/simulation.test_1_2.dat
######################################################################
set -e
WRAPPERDIR=`dirname $0`

########### configuration #############
maxindex=6
cpu=80
io=100
# memory per offset class
memory=16000
# edge expression of output data related to memory consumption
edgeExpression=Trace
#######################################

function usage {
  echo "### USAGE: getNextTrace.sh -OffsetClass <OFFSET_CLASS_FILE> -hasNext <true|false> -i 0 -Trace <TRACE_OUTPUT_FILE> -hasNext <HASNEXT_OUTPUT_FILE> [-simulation <SIMULATION_LOG_FILE>]"
}

INDEX=1
while [ $# -gt 0 ]
do
  case $1 in
    -OffsetClass)
      offsetclassfn=$2
      shift 2
      ;;
    -hasNext)
      hasnext=$2
      echo "# hasnext=$hasnext"
      shift 2
      ;;
    -i)
      index=$2
      echo "# i=$index"
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

if [ -z "$offsetclassfn" ]; then
  echo "### ERROR: missing input parameter '-OffsetClass'!" 1>&2
  usage
  exit 2
fi

if [ ! -r "$offsetclassfn" ]; then
  echo "### ERROR: cannot read input file '$offsetclassfn'!" 1>&2
  usage
  exit 3
fi
  
if [ -z "$hasnext" ]; then
  echo "### ERROR: missing input/output parameter '-hasNext'!" 1>&2
  usage
  exit 4
fi

if [ -z "$index" ]; then
  echo "### ERROR: missing input parameter '-i'!" 1>&2
  usage
  exit 5
fi

if [ -z "$tracefn" ]; then
  echo "### ERROR: missing output parameter '-Trace'!" 1>&2
  exit 6
fi

####################################
offsetclass=`cat $offsetclassfn | grep offsetclass`
echo "# offsetclass=$offsetclass"
echo "# memory=$memory"

if [ $index -gt $maxindex ]; then
  echo "ERROR: index 'i' not within range [0,$maxindex]!" 1>&2
  usage
  exit 5
fi

if [ $index -lt $maxindex ]; then
  echo "<hasnext>true</hasnext>" > $hasnext
else 
  echo "<hasnext>false</hasnext>" > $hasnext
fi

echo "$offsetclass" > $tracefn
echo "<traceindex>$index</traceindex>" >> $tracefn
echo "<simulation><memory><fvm>$memory</fvm></memory></simulation>" >> $tracefn

if [ -n "$simulation" ]; then
  echo "<simulation><duration><cpu>$cpu<cpu><io>$io</io><memory><fvm edgeExpression=\"$edgeExpression\">$memory</fvm></memory></simulation>" > $simulation
fi
