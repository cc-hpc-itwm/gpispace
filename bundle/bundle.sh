#!/bin/bash

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
  return 1
}

function bundle_dependencies ()
{
  local file="$1" ; shift
  local dst="$1"; shift
  OLDIFS="$IFS"
  export IFS="
"
  for dep_and_path in $(ldd "$file" | grep '=>' | grep '/' | awk '{printf("%s:%s\n", $1, $3)}') ; do
     dep=$(echo "$dep_and_path" | cut -d: -f 1)
     pth=$(echo "$dep_and_path" | cut -d: -f 2)
     if is_filtered "$dep" ; then
	 continue
     fi
#     path=$(locate_file "$dep")
     path="$pth"
     if [ -n "$path" ] ; then
        if [ ! -e "$dst/$dep" ] ; then
          echo >&2 cp "$path" "$dst/$dep"
          cp "$path" "$dst/$dep"
          echo "$path"
          bundle_dependencies "$path" "$dst"
        fi
     fi
  done
  IFS="$OLDIFS"
}

for bin ;  do
   echo >&2 "# bundling dependencies for '$bin'"
   bundle_dependencies "$bin" "$CMAKE_INSTALL_PREFIX/lib"
done
