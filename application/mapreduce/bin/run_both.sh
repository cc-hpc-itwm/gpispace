#/bin/sh!

list="100 200"

cd ${HOME}/hadoop-1.1.2
./run_hadoop_tests.sh "$list"
stop-all.sh
cd ${HOME}/git/sdpa/application/mapreduce/xml
./run_sdpa_tests.sh "$list"
sdpa stop
