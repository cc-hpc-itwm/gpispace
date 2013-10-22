#!/bin/sh

N=$(cat $PBS_NODEFILE | wc -l)
RAM=$(free|awk '/^Mem:/{print $2}')

echo "RAM: $RAM"
let GPIALLOC=$N*$RAM*1/2

echo "Total memory to be allocated for GPI: $GPIALLOC"

list="11000"

if [ -n "$1" ]; then list="$1"; fi

echo "list=$list"

cd ../xml

for k in $list ; do
        sdpa stop
	command_rm="rm *.pnet.put *.out"
	echo $command_rm
	$command_rm

	sleep 30
	sdpa boot -m GPIALLOC -s  2*2**30  INI#-1:1x1,0 FIN#-1:1x1,0 LOAD#-1:4,2**30 MAP+RED#-1:10 RED#-1:2
	#original: 
        #sdpa boot -m GPIALLOC -s  2*2**30  INI#-1:1x1,0 FIN#-1:1x1,0 LOAD+RED#-1:7,2**30 MAP#-1:5
	sleep 30

        echo "Run mapreduce on words.$k"
        cat ../bin/Makefile_ml.tpl | sed -e "s/2000/$k/g" > Makefile

        command_run="make submit"
        START=$(date +%s)
        echo $command_run
        $command_run
        END=$(date +%s)
        DIFF=$(( $END - $START ))
        echo "SDPA: time words.$k:  $DIFF seconds"
        SUFFIX=$(date '+%d.%m.%y')
        echo "SDPA: time words.$k:  $DIFF seconds" >> ../test_results/sdpa_exec_time_${N}n_${SUFFIX}.txt

done
date

