#!/bin/bash

prefix=/usr/local
exclude=()
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

function is_filtered_by ()
{
    for ((p=0 ; p < ${#exclude[@]}; ++p)) ; do
        if echo $(basename "$1") | grep -q "${exclude[$p]}" ; then
	    return $p
        fi
    done
    return ${#exclude[@]}
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
	is_filtered_by "$dep"
	res_pattern=$?
        if [ $res_pattern -lt ${#exclude[@]} ] ; then
            debug $(printf "%${indent}s" "") "$file >- $pth  (filtered by pattern: '${exclude[$res_pattern]}')"
            continue
	else
            debug $(printf "%${indent}s" "") "$file <- $pth"
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
while getopts ":hvnkfp:x:o:" opt ; do
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
	o)
	    dst=$OPTARG
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
