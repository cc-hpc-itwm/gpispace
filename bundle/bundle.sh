#!/bin/bash

prefix=/usr/local
exclude=()
verbose=false
dry=false
force=false
keep_going=false

function debug ()
{
    $verbose && echo >&2 $@
}

function dry_run ()
{
    $dry && echo $@ || $@ || $keep_going || exit 1
}

function locate_file ()
{
  local file="$1" ; shift
  OLDIFS="$IFS"
  export IFS=":"
  for dir in $LD_LIBRARY_PATH ; do
    if [ -e "$dir/$file" ] ; then
       echo "$dir/$file"
       break
    fi
  done
  IFS="$OLDIFS"
}

function is_filtered ()
{
    for pattern in ${exclude[@]} ; do
        if echo "$1" | grep -q "$pattern" ; then
            return 0
        fi
    done
    return 1
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
    for dep_and_path in $(ldd "$file" | grep '=>' | grep '/' | awk '{printf("%s:%s\n", $1, $3)}') ; do
        dep=$(echo "$dep_and_path" | cut -d: -f 1)
        pth=$(echo "$dep_and_path" | cut -d: -f 2)
        debug $(printf "%${indent}s" "") "$file <- $pth"
        if is_filtered "$dep" ; then
            debug "dependency '$dep' was filtered by pattern '$pattern'"
            continue
        fi
        path="$pth"
        if [ -n "$path" ] ; then
            if ! test -e "$dst/$dep" || $force ; then
                debug $(printf "%$((indent + 2))s" "") cp "$path" "$dst/$dep"
                dry_run cp "$path" "$dst/$dep"
                bundle_dependencies "$path" "$dst" $(( lvl + 1 ))
            fi
        fi
    done
    IFS="$OLDIFS"
}

shiftcount=0
while getopts ":hvnkfp:x:" opt ; do
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
            exclude=( ${exclude[@]} $OPTARG )
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

dry_run mkdir -p "$prefix/lib"

for bin ;  do
   echo >&2 "# bundling dependencies for '$bin'"
   bundle_dependencies "$bin" "$prefix/lib"
done
