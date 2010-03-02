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
# Script for testing dummy application scripts for pstm.
#
# @author Andreas Hoheisel
# @version $Id$
#
######################################################################
set -e
WRAPPERDIR=`dirname $0`

rm -rf $WRAPPERDIR/test/*.dat
$WRAPPERDIR/test_getNumberOfPoints.sh
$WRAPPERDIR/test_getNextOffsetClass.sh -i 0
$WRAPPERDIR/test_getNextOffsetClass.sh -i 1
$WRAPPERDIR/test_getNextOffsetClass.sh -i 2
$WRAPPERDIR/test_getNextOffsetClass.sh -i 3
$WRAPPERDIR/test_getNextOffsetClass.sh -i 4
$WRAPPERDIR/test_getNextOffsetClass.sh -i 5
$WRAPPERDIR/test_getNumberOfPoints.sh
$WRAPPERDIR/test_createOutputGather.sh
$WRAPPERDIR/test_createVolume.sh -i 0
$WRAPPERDIR/test_createVolume.sh -i 1
$WRAPPERDIR/test_createVolume.sh -i 2
$WRAPPERDIR/test_createVolume.sh -i 3
$WRAPPERDIR/test_createVolume.sh -i 4
$WRAPPERDIR/test_createVolume.sh -i 5
$WRAPPERDIR/test_getNextTrace.sh -i 0 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 0 -j 1
$WRAPPERDIR/test_getNextTrace.sh -i 1 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 1 -j 1
$WRAPPERDIR/test_getNextTrace.sh -i 2 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 2 -j 1
$WRAPPERDIR/test_getNextTrace.sh -i 3 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 3 -j 1
$WRAPPERDIR/test_getNextTrace.sh -i 4 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 4 -j 1
$WRAPPERDIR/test_getNextTrace.sh -i 5 -j 0
$WRAPPERDIR/test_getNextTrace.sh -i 5 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 0 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 0 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 1 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 1 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 2 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 2 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 3 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 3 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 4 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 4 -j 1
$WRAPPERDIR/test_addTraceContribution.sh -i 5 -j 0
$WRAPPERDIR/test_addTraceContribution.sh -i 5 -j 1
$WRAPPERDIR/test_store.sh -i 0
$WRAPPERDIR/test_store.sh -i 1
$WRAPPERDIR/test_store.sh -i 2
$WRAPPERDIR/test_store.sh -i 3
$WRAPPERDIR/test_store.sh -i 4
$WRAPPERDIR/test_store.sh -i 5
