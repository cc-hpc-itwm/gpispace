#!/bin/bash

dst="${1:?}"

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
    OLDIFS="$IFS"
    export IFS="
"

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
                bundle_dependencies "$pth"
            fi
        fi
    done
    IFS="$OLDIFS"
}

mkdir -p "$dst" || exit 1

bundle_dependencies "${2:?}"
