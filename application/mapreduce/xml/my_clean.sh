#!/bin/sh

sdpa stop
#clean 
command_rm="rm *.pnet.put *.out"
echo $command_rm
$command_rm

sleep 30
#boot the system
./sdpa_mapred_boot_seislab.sh
sleep 30
