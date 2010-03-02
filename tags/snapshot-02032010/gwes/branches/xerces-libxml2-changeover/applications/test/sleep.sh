#!/bin/bash
######################################################################
# $Id$
#
# Copyright (c) 2002, FhRG Project Team, Fraunhofer FIRST
# (Fraunhofer-Institut für Rechnerarchitektur und Softwaretechnik)
# See http://www.fhrg.fhg.de for more details.
#
######################################################################
#
# Script for wrapping sleep
#
# @author Andreas Hoheisel
# @version $Id$
#
######################################################################
SLEEP=`which sleep`

function usage {
  echo "### USAGE: sleep.sh -duration <DURATION_SECONDS>"
}

while [ $# -gt 1 ]
do
  case $1 in
    -duration)
      duration=$2
      shift 2
      ;;
    *)
      echo "### Error: unknown argument $1 ###"
      usage
      exit 1
      shift
      ;;
  esac
done

if [ -n "$duration" ]; then
  exec ${SLEEP} $duration
else
  echo "### ${SLEEP} $duration"
  echo "### Error: the duration has not been specified! ###"
  usage
  exit 1
fi
