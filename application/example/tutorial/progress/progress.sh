#!/bin/bash

app_name="$1"
if [ -z "$app_name" ] ; then
    echo >&2 "usage: $(basename $0) progress-app-name"
    exit 1
fi

fhgkvsc=$(which fhgkvsc)
if [ $? -ne 0 ] ; then
    echo >&2 "fhgkvsc not found in PATH, please load sdpa"
fi

phase=$(${fhgkvsc} -f -g "progress.${app_name}.phase" -v "n/a" 2>/dev/null)

maximum=$(${fhgkvsc} -f -g "progress.${app_name}.maximum" -v 100 2>/dev/null)
if [ $? -ne 0 ] ; then
    echo >&2 "could not get progress maximum"
    exit 3
fi

while true ; do
    current=$(${fhgkvsc} -f -g "progress.${app_name}.current" -v 0)
    echo -n "$app_name ($phase): "
    ./show-progress.sh $current $maximum '\r'
    if [ $current -eq $maximum ] ; then
        break
    fi
    sleep 0.5
done
echo
