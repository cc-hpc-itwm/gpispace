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

# error codes:
ESUC=0
EERR=1
EGPI=4

function log()
{
    echo "$*" >&2
    echo "$*" >> ${logfile}
}

function is_alive ()
{
    name=$1
    pid=$2
    kill -0 $pid >/dev/null 2>&1
    if [ $? -eq 0 ]; then
	return 0
    else
	return 1
    fi
}

:> ${logfile}
log "initializing selftest..."
log "starting sdpa-gpi..."
${sdpa_gpi} >> ${logfile} 2>&1 &
sdpa_gpi_pid=$!
sleep 2
if ! is_alive "sdpa_gpi" $sdpa_gpi_pid ; then
    echo "SDPA-GPI interface could not be started!"
    exit ${EGPI}
fi
