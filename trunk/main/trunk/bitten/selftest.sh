#!/bin/sh
#
# a small tool to test the whole SDPA installation for functionality
#
sdpa_gpi="/opt/cluster/GPI/bin/petry/sdpa-gpi"
orch="orchestrator"
agg="aggregator"
nre="nre"
pcd="nre-pcd"
pnetc="pnetc"
sdpac="sdpac"
job="stresstest.xml"

logfile=/tmp/sdpa-selftest.log

# error codes:
ESUC=0
EERR=1
EGPI=4
EORCH=5
EAGG=6
ENRE=7
EPCD=8
ESUB=9

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

function shutdown()
{
  log "shutting everything down..."
  for pid in `jobs -p` ; do
	log "shutting down $pid..."
	kill -TERM $pid >/dev/null 2>&1
	sleep 1
	if is_alive $pid $pid ; then
	  log "killing $pid..."
	  kill -KILL $pid >/dev/null 2>&1
    fi
  done
  log "waiting for background processes..."
  wait
}


trap shutdown EXIT

:> ${logfile}
log "initializing selftest..."
log "starting sdpa-gpi..."
${sdpa_gpi} >> ${logfile} 2>&1 &
sdpa_gpi_pid=$!
sleep 2
if ! is_alive "sdpa_gpi" $sdpa_gpi_pid ; then
    log "E: SDPA-GPI interface could not be started!"
    exit ${EGPI}
else
	log "OK"
fi

log "starting orchestrator..."
${orch} >> ${logfile} 2>&1 &
orch_pid=$!
sleep 1
if ! is_alive "orchestrator" $orch_pid ; then
	log "E: Orchestrator could not be started!"
	exit ${EORCH}
else
	log "OK"
fi

log "starting aggregator..."
${agg} >> ${logfile} 2>&1 &
agg_pid=$!
sleep 1
if ! is_alive "aggregator" $agg_pid ; then
	log "E: Aggregator could not be started!"
	exit ${EAGG}
else
	log "OK"
fi

fvm_pc_cfg="/p/herc/itwm/hpc/soft/sdpa/fvm-pc/etc/fvm.cfg"
if [ -n "$SDPA_FVM_PC_CFG" ]; then
  fvm_pc_cfg="$SDPA_FVM_PC_CFG"
fi

log "reading PCD config from $fvm_pc_cfg..."
while read k v; do
    case "$k" in
        SHMSZ)
            log "using shmsz $v"
            export FVM_PC_SHMSZ="$v"
            ;;
        FVMSZ)
            log "using fvmsz $v"
            export FVM_PC_FVMSZ="$v"
            ;;
        MSQFILE)
            log "using msq $v"
            export FVM_PC_MSQ="$v"
            ;;
        SHMFILE)
            log "using shm $v"
            export FVM_PC_SHM="$v"
            ;;
    esac
done < "$fvm_pc_cfg"

log "starting process-container..."
${pcd} --rank "0" --load "/p/herc/itwm/hpc/soft/sdpa/ap/gcc/libexec/libfvm-pc.so" -a /p/herc/itwm/hpc/soft/sdpa/ap/gcc/libexec >> ${logfile} 2>&1 &
pcd_pid=$!
sleep 1
if ! is_alive "pcd" $pcd_pid ; then
	log "E: PCD could not be started!"
	exit ${EPCD}
else
	log "OK"
fi

log "starting nre..."
${nre} >> ${logfile} 2>&1 &
nre_pid=$!
sleep 1
if ! is_alive "nre" $nre_pid ; then
	log "E: NRE could not be started!"
	exit ${ENRE}
else
	log "OK"
fi
log "submitting job..."
jobid=$( ${sdpac} submit selftest.pnet )
log "JOB-ID := $jobid"
log "waiting for job to return..."
${sdpac} wait $jobid
