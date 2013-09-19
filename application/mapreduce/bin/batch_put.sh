#!/bin/sh

list="110000 100000 90000 80000 70000 60000 50000 45000 40000 35000 30000 25000 20000 15000 10000 9000 8000 7000 6500 6000 5500 5000 4500 4000 3500 3000 2500 2000 1500 1000 500"
for k in $list ; do
        command_put="hadoop fs -put /scratch/rotaru/data/words.$k /words.$k"
	START=$(date +%s)
        echo $command_put
        $command_put
        END=$(date +%s)
        DIFF=$(( $END - $START ))
        echo "Loading the input file /scratch/rotaru/data/words.$k into HDFS took:  $DIFF seconds"
	echo "Haddop: time words.$k:  $DIFF seconds" >> hadoop_load_time_$SUFFIX.txt

	command_run="hadoop jar hadoop-examples-*.jar wordcount /words.$k /output_words_$k"
	START=$(date +%s)
        echo $command_run
        $command_run
        END=$(date +%s)
	DIFF=$(( $END - $START ))
	echo "SDPA: time words.$k:  $DIFF seconds"
        SUFFIX=$(date '+%m_%d_%y')
        echo "Haddop: time words.$k:  $DIFF seconds" >> hadoop_exec_time_$SUFFIX.txt

        command_rm="hadoop fs -rmr /words.$k"
        echo $command_rm
        $command_rm
done
date
stop-all.sh
