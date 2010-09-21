#!/bin/sh
#
# a small tool to test the whole SDPA installation for functionality
#
sdpa_gpi="/opt/cluster/GPI/bin/petry/sdpa-gpi"
orch="orchestrator"
agg="aggregator"
nre="nre"
pcd="nre-pcd"

logfile=/tmp/sdpa-selftest.log


function log()
{
    echo "$*" >&2
    echo "$*" >> ${logfile}
}

log test 1 2 3 4
