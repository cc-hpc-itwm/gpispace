#!/bin/bash

bar_name="$1"
if [ -z "$bar_name" ] ; then
	echo >&2 "usage: $(basename $0) progress-bar-name"
	exit 1
fi

fhgkvsc=$(which fhgkvsc)
if [ $? -ne 0 ] ; then
  echo >&2 "fhgkvsc not found in PATH, please load sdpa"
fi

maximum=$(${fhgkvsc} -f -g progress.${bar_name}.maximum -v 100 2>/dev/null)
if [ $? -ne 0 ] ; then
	echo >&2 "could not get progress maximum"
	exit 3
fi

while true ; do
	current=$(${fhgkvsc} -f -g progress.${bar_name}.current)
	if [ $? -ne 0  ] ; then
		echo >&2 "no such progress available: $bar_name"
		exit 2
	fi
	while [ $current -ne $maximum ] ; do
	  printf "%s: " $bar_name
	  ./show-progress.sh $current $maximum '\r'
	  sleep 0.5
	  current=$(${fhgkvsc} -f -g progress.${bar_name}.current)
	done
done

exit 0
