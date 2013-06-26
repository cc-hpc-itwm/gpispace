#!/bin/sh
#sdpa boot -G 2 -m 32*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:4 RED:8

N=$(cat $PBS_NODEFILE | wc -l)
RAM=$(free|awk '/^Mem:/{print $2}')

echo "RAM: $RAM"
let GPIALLOC=$N*$RAM*1/2

echo "Total memory to be allocated for GPI: $GPIALLOC"

list="110000"
#"100000 90000 80000 70000 60000 50000 40000 30000 20000 10000 5000"
#"10000 20000 30000 40000 50000 60000 70000 80000 90000 100000 110000 120000 130000 140000 150000 160000 170000 180000 190000 200000 210000"

if [ -n "$1" ]; then list="$1"; fi

echo "list=$list"

for k in $list ; do
        sdpa stop
	command_rm="rm *.pnet.put *.out"
	echo $command_rm
	$command_rm

	sleep 30
	#sdpa boot -m GPIALLOC -s  2*2**30  INI#-1:1x1,0 FIN#-1:1x1,0 LOAD+RED#-1:4 MAP+RED#-1:8 #-> 4428s with split
        #add MAP#2x1,2**30 !!!!!!!!!!!!!!!!!!!!!!!!!!!
	sdpa boot -m GPIALLOC -s  2*2**30  INI#-1:1x1,0 FIN#-1:1x1,0 LOAD+RED#-1:7,2**30 MAP#-1:5 #words.11000 -> 348s
	sleep 30

        echo "Run mapreduce on words.$k"
        cat Makefile_ml.tpl.fast | sed -e "s/2000/$k/g" > Makefile

        command_run="make submit"
        START=$(date +%s)
        echo $command_run
        $command_run
        END=$(date +%s)
        DIFF=$(( $END - $START ))
        echo "SDPA: time words.$k:  $DIFF seconds"
        SUFFIX=$(date '+%d.%m.%y')
        echo "SDPA: time words.$k:  $DIFF seconds" >> sdpa_exec_time_${N}n_${SUFFIX}.txt

	#sleep 30
done
date

