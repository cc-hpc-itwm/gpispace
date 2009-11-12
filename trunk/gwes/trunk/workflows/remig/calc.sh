#!/bin/bash

while [ ${#} -gt 0 ]
do
  case ${1} in
    -slice_and_depth)
     slice_and_depth=$((${2:?}))
     ;;
    # get it as file, special to the commandline activity, i don't know why
    -number_of_frequencies)
     number_of_frequencies=$((`cat ${2:?} | sed 's!<data>!!;s!</data>!!;s!<sdpa>!!;s!</sdpa>!!'`))
     ;;
    -memhandle_for_configuration)
     memhandle_for_configuration=`cat ${2:?} | sed 's!<data>!!;s!</data>!!;s!<sdpa>!!;s!</sdpa>!!'`
     ;;
    *)
     echo "wrong parameters" 1>&2
  esac
  shift 2
done

#echo "slice_and_depth = ${slice_and_depth}"
#echo "number_of_frequencies = ${number_of_frequencies}"

slice=$((slice_and_depth / number_of_frequencies))
depth=$((slice_and_depth % number_of_frequencies))

echo "slice = ${slice} depth = ${depth} memhandle_for_configuration = ${memhandle_for_configuration}"
