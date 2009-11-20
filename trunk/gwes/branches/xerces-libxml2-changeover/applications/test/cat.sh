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
# Script for wrapping cat
#
# @author Andreas Hoheisel
# @version $Id$
#
######################################################################
CAT=`which cat`

function usage {
  echo "### USAGE: cat.sh -input1 <FILENAME1> -input2 <FILENAME2>"
}

while [ $# -gt 1 ]
do
  case $1 in
    -input1)
      input1=$2
      shift 2
      ;;
    -input2)
      input2=$2
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

if [ -r "$input1" -a -r "$input2" ]; then
  exec ${CAT} $input1 $input2
else
  echo "### ${CAT} $input1 $input2"
  echo "### Error: the input files are not specified or not available! ###"
  usage
  exit 1
fi
