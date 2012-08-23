#!/bin/bash

app_name="$1"
if [ -z "$app_name" ] ; then
#    echo >&2 "W: usage: $(basename $0) progress-app-name"
#    echo >&2 "   using: progress-example"
    app_name="progress-example"
fi

function show_progress ()
{
  local cur="$1"
  local max="$2"
  local term="$3"
  
  if [ -z "$max" ] ; then
  	max=100
  fi
  
  percent_done=$(echo "scale=4; ${cur}/${max} * 100." | bc -l)
  
  percent_marker=$(printf "%*.0f%%" 3 $percent_done)
  
  screen_width=80
  bar_width=$(( $screen_width - 4 - 1 - 2 )) # 4 == percent, 1 == space, 2 == []
  columns_filled=$(echo "scale=2; $percent_done/100.0 * $bar_width" | bc -l | xargs printf "%.0f")
  
  echo -n "["
  i=0
  while [ $i -lt $columns_filled ] ; do
    echo -n "="
    i=$(( i + 1 ))
  done
  while [ $i -lt $bar_width ] ; do
    echo -n " "
    i=$(( i + 1 ))
  done
  printf "] %s" "$percent_marker"
  printf "%s${term}"
}

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
    show_progress $current $maximum '\r'
    if [ $current -eq $maximum ] ; then
        break
    fi
    sleep 0.5
done
echo
