#!/bin/bash
while [ ${#} -gt 0 ]
do
  case ${1} in
    -slice_and_depth)
     slice_and_depth=${2:?}
     shift 2
     ;;
    -number_of_frequencies)
     number_of_frequencies=${2:?}
     shift 2
     ;;
    *)
     echo "wrong parameters" 1>&2
     exit 2
  esac
done

#echo "slice_and_depth = ${slice_and_depth}"
#echo "number_of_frequencies = ${number_of_frequencies}"

slice=$((slice_and_depth / number_of_frequencies))
depth=$((slice_and_depth % number_of_frequencies))

echo "slice = ${slice} depth = ${depth}"
