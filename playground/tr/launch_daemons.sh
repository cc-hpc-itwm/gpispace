#!/bin/bash

num_nre=1
if [ $# -gt 0 ]
then
   num_nre=$1
   shift
fi

for d in orchestrator aggregator nre
do
	echo -n "killing $d..."
	pkill "$d"
	sleep 1
	echo ""
done

for d in orchestrator aggregator
do
	rm -f "$d.log"
	echo -n "starting daemon $d..."
	FHGLOG_level=MIN ./$d > $d.log 2>&1 &
	sleep 1
	echo "done."
done

echo -n "starting PCDs"...
./run-pcd "$num_nre"
sleep 1
echo ""

for i in $(seq 0 $( expr $num_nre - 1 ) )
do
        rm -f "nre.$i.log"
	nre_port=$( expr $i + 5002 )
	worker_port=$( expr $i + 8000 )
	#  -n [ --name ] arg                         NRE's logical name
	#  -u [ --url ] arg (=127.0.0.1:5002)        NRE's url
	#  -m [ --agg_name ] arg (=aggregator_1)     Aggregator's logical name
	#  -p [ --agg_url ] arg (=127.0.0.1:5001)    Aggregator's url
	#  -w [ --worker_url ] arg (=127.0.0.1:8000) Worker's url

	echo -n "starting NRE_$i..."
	FHGLOG_level=MIN ./nre -n "NRE_${i}" -u 127.0.0.1:$nre_port -m "aggregator_0" -p "127.0.0.1:5001" -w 127.0.0.1:$worker_port > nre.$i.log 2>&1 &
	sleep 1
done

echo -n "waiting for subprocesses to finish..."
wait
echo
