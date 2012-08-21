#!/bin/bash

cur="$1"
max="$2"
term="$3"

name=$(basename "$0")

if [ -z "$cur" ] ; then
	echo >&2 "usage: $name current [max]"
	echo >&2 "    default maximum is 100"
	exit 1
fi

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
