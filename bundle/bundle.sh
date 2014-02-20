#!/bin/bash

prefix=/usr/local
exclusion=""
inclusion=""
verbose=false
dry=false
force=false
keep_going=false
dst=lib # folder within prefix where libs shall be copied to
delete=false
library_path="$LD_LIBRARY_PATH"
copied=()

function usage ()
{
    cat <<EOF
usage: $(basename $0) [options]

  -h : print this help
  -v : be verbose
  -n : dry run
  -k : keep going in case of errors
  -f : force (overwrite existing files)
  -d : delete filtered files from target
  -p : installation prefix (=$prefix)
  -x : exclude pattern (can occur multiple times)
  -w : include pattern (can occur multiple times)
  -L : library path (can occur multiple times)

  -o  : output destination  folder  for  dependencies, if  not  absolute,  interpreted
        as a relative to <prefix> (=$dst)
EOF
}

function debug ()
{
    $verbose && echo >&2 $@
}

function dry_run ()
{
    $dry && echo $@ || $@ || $keep_going || exit 1
}

function is_filtered ()
{
    local name=$(basename "$1")

    if [ -n "${exclusion}" ]
    then
        if echo "$name" | grep -q "${exclusion}"
        then
            return 0
        fi
    fi

    if [ -n "${inclusion}" ]
    then
        if echo "$name" | grep -q "${inclusion}"
        then
            return 1
        else
            return 0
        fi
    fi

    return 1
}

function is_copied_already()
{
    local name="${1}"; shift;

    for c in ${copied[@]}
    do
        test "${c}" = "${name}" && return 0
    done

    return 1
}

function normalize()
{
    local name="$1"; shift

    readlink -f "${name}"
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
    for dep_and_path in $(ldd "$file" | grep '=> \(not\|/\)' | awk '{printf("%s:%s\n", $1, $3)}') ; do
        dep=$(echo -n "$dep_and_path" | cut -d: -f 1)
        pth=$(echo -n "$dep_and_path" | cut -d: -f 2)

        if [ "$pth" = "not" ] ; then
            echo >&2 "cannot resolve dependency: '$dep' of file '$file'"
            echo >&2 "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
            if ! $keep_going ; then
                exit 2
            fi
        fi

        if is_filtered "$dep" ; then
            debug $(printf "%${indent}s" "") "$file >- $pth  (filtered)"

            if $delete && test -e "$dst/$dep"
            then
                debug $(printf "%${indent}s" "") "rm $dst/$dep"
                dry_run rm -f "$dst/$dep"
            fi
            continue
        else
            debug $(printf "%${indent}s" "") "$file <- $pth"
        fi

        pth=$(normalize "${pth}")
        tgt=$(normalize "$dst/$dep")

        if is_copied_already "$pth"
        then
            debug $(printf "%$((indent + 2))s" "") "already copied: ${pth}"
        else
        if [[ ! "$pth" = "$tgt" ]] ; then
            if test "$pth" -nt "$tgt" || $force ; then
                debug $(printf "%$((indent + 2))s" "") cp "$pth" "$tgt"
                dry_run cp "$pth" "$tgt"
                copied+=("$pth")
                bundle_dependencies "$pth" "$dst" $(( lvl + 1 ))
            fi
        fi
        fi
    done
    IFS="$OLDIFS"
}

shiftcount=0
while getopts ":hvnkfp:x:w:o:dL:" opt ; do
    case $opt in
        h)
            usage
            exit 0
            ;;
        v)
            verbose=true
            shiftcount=$(( shiftcount + 1 ))
            ;;
        n)
            dry=true
            shiftcount=$(( shiftcount + 1 ))
            ;;
        k)
            keep_going=true
            shiftcount=$(( shiftcount + 1 ))
            ;;
        f)
            force=true
            shiftcount=$(( shiftcount + 1 ))
            ;;
        p)
            prefix=$OPTARG
            shiftcount=$(( shiftcount + 2 ))
            ;;
        x)
            if [ -z "${exclusion}" ]
            then
                exclusion="$OPTARG"
            else
                exclusion="$exclusion\|$OPTARG"
            fi
            shiftcount=$(( shiftcount + 2 ))
            ;;
        w)
            if [ -z "${inclusion}" ]
            then
                inclusion="$OPTARG"
            else
                inclusion="$inclusion\|$OPTARG"
            fi
            shiftcount=$(( shiftcount + 2 ))
            ;;
        o)
            dst=$OPTARG
            shiftcount=$(( shiftcount + 2 ))
            ;;
        d)
            delete=true
            shiftcount=$(( shiftcount + 1 ))
            ;;
        L)
            if [ -z "${library_path}" ] ; then
                library_path="$OPTARG"
            else
                library_path="${library_path}:$OPTARG"
            fi
            shiftcount=$(( shiftcount + 2 ))
            ;;
        \?)
            echo "invalid option: -$OPTARG" >&2
            echo "try: $(basename $0) -h" >&2
            exit 1
            ;;
    esac
done

shift $shiftcount

case "$dst" in
    /*)
        :
        ;;
    *)
        dst="$prefix/$dst"
        ;;
esac

dry_run mkdir -p "$dst"

export LD_LIBRARY_PATH="${library_path}"

for bin ;  do
   debug "# bundling dependencies for '$bin'"
   bundle_dependencies "$bin" "$dst"
done
