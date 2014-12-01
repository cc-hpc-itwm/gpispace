#!/bin/bash

dst=

function join { local IFS="$1"; shift; echo "$*"; }
exclusion=$(join '|' \
  libibverbs.* \
  libxcb.* \
  libSM.* \
  libc.so.* \
  libz.so.* \
  libm.so.* \
  librt.* \
  libfont.* \
  libfreetype.* \
  libaudio.* \
  libICE.* \
  libglib.* \
  libgobject.* \
  libdl.* \
  libX.*so \
  libpthread.* \
  libgthread.* \
  libreadline.* \
  libboost*.*
)

function is_filtered ()
{
  basename "$1" | grep -qE "${exclusion}"
}

function bundle_dependencies ()
{
    local file="$1" ; shift
    local dst="$1"; shift
    local lvl="${1:-0}"; shift
    local indent=$(( lvl * 4 ))
    OLDIFS="$IFS"
    export IFS="
"

    echo "-- Bundle: Examining $file"

    for dep_and_path in $(ldd "$file" | grep '=> \(not\|/\)' | awk '{printf("%s:%s\n", $1, $3)}') ; do
        dep=$(echo -n "$dep_and_path" | cut -d: -f 1)
        pth=$(echo -n "$dep_and_path" | cut -d: -f 2)

        if is_filtered "$dep" ; then
            if test -e "$dst/$dep"
            then
                rm -f "$dst/$dep" || exit 1
            fi
            continue
        fi

        if [ "$pth" = "not" ] ; then
            echo >&2 "cannot resolve dependency: '$dep' of file '$file'"
            echo >&2 "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
            exit 2
        fi

        pth=$(readlink -f "${pth}")
        tgt=$(readlink -f "$dst/$dep")

        if [[ ! "$pth" = "$tgt" ]] ; then
            if test "$pth" -nt "$tgt" ; then
                echo "-- Installing: Bundle: $tgt"
                cp "$pth" "$tgt" || exit 1
                bundle_dependencies "$pth" "$dst" $(( lvl + 1 ))
            else
                echo "-- Up-to-date: Bundle: $tgt"
            fi
        fi
    done
    IFS="$OLDIFS"
}

shiftcount=0
while getopts ":hvnkfp:x:w:o:dL:" opt ; do
    case $opt in
        o)
            dst=$OPTARG
            shiftcount=$(( shiftcount + 2 ))
            ;;
        \?)
            echo "invalid option: -$OPTARG" >&2
            echo "try: $(basename $0) -h" >&2
            exit 1
            ;;
    esac
done

if [ -z "$dst" ]; then
  echo "missing destination (-o)" >&2
  exit 1
fi

shift $shiftcount

mkdir -p "$dst" || exit 1

for bin ;  do
   bundle_dependencies "$bin" "$dst"
done
