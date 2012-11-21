#!/bin/bash

prefix=/usr/local
exclude=()
whitelist=()
verbose=false
dry=false
force=false
keep_going=false
dst=lib # folder within prefix where libs shall be copied to

function usage ()
{
    cat <<EOF
usage: $(basename $0) [options]

  -h : print this help
  -v : be verbose
  -n : dry run
  -k : keep going in case of errors
  -f : force (overwrite existing files)
  -p : installation prefix (=$prefix)
  -x : exclude pattern (can occur multiple times)
  -w : include pattern (can occur multiple times)

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

function is_in_whitelist ()
{
    local name="$1"
    for ((p=0 ; p < ${#whitelist[@]}; ++p)) ; do
        if echo "$name" | grep -q "${whitelist[$p]}"
        then
            return 0
        fi
    done
    return 1
}

function is_in_blacklist ()
{
    local name="$1"
    for ((p=0 ; p < ${#exclude[@]}; ++p)) ; do
        if echo "$name" | grep -q "${exclude[$p]}" ; then
            return 0
        fi
    done
    return 1
}

function is_filtered ()
{
    local name=$(basename "$1")

    if is_in_blacklist "$name"
    then
        return 0
    fi

    if [ ${#whitelist[@]} -gt 0 ]
    then
        if is_in_whitelist "$name"
        then
            return 1
        else
            return 0
        fi
    fi

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
        if is_filtered "$dep" ; then
            debug $(printf "%${indent}s" "") "$file >- $pth  (filtered)"
            continue
        else
            debug $(printf "%${indent}s" "") "$file <- $pth"
        fi
        path="$pth"
        if [ -n "$path" ] ; then
            if test "$path" -nt "$dst/$dep" || $force ; then
                debug $(printf "%$((indent + 2))s" "") cp "$path" "$dst/$dep"
                dry_run cp "$path" "$dst/$dep"
                bundle_dependencies "$path" "$dst" $(( lvl + 1 ))
            fi
        fi
    done
    IFS="$OLDIFS"
}

shiftcount=0
while getopts ":hvnkfp:x:w:o:d" opt ; do
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
        w)
            whitelist=( ${whitelist[@]} $OPTARG )
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

for bin ;  do
   echo >&2 "# bundling dependencies for '$bin'"
   bundle_dependencies "$bin" "$dst"
done
