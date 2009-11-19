#!/bin/bash
while [ ${#} -gt 0 ]
do
  case ${1} in
    -config_file)
     config_file=${2:?}
     ;;
    -number_of_frequencies)
     number_of_frequencies=${2:?}
     ;;
    -number_of_depthlevels)
     number_of_depthlevels=${2:?}
     ;;
    -number_of_parallel_propagators)
     number_of_parallel_propagators=${2:?}
     ;;
    -memhandle_for_outputvolume)
     memhandle_for_outputvolume=${2:?}
     ;;
    -memhandle_for_configuration)
     memhandle_for_configuration=${2:?}
     ;;
    -seq)
     seq=${2:?}
     ;;
    *)
     echo "wrong parameters" 1>&2
     exit 2
  esac
  shift 2
done

# attention! using the commandline activities, there will be
# *a lot* of files created, when using high numbers here!

echo "<data><sdpa>10</sdpa></data>" > ${number_of_frequencies}
echo "<data><sdpa>5</sdpa></data>" > ${number_of_depthlevels}
echo "<data><sdpa>3</sdpa></data>" > ${number_of_parallel_propagators}
echo "<data><sdpa>0xdeadbeef</sdpa></data>" > ${memhandle_for_outputvolume}
echo "<data><sdpa>0x47110815</sdpa></data>" > ${memhandle_for_configuration}
echo "<data><sdpa>SEQ</sdpa></data>" > ${seq}
