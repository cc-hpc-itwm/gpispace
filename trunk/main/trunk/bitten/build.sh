#!/bin/sh
# configures and builds a specific build

if [ $# -ne 3 ]; then
  echo "usage: $0 <type> <path-src> <path-build>"
  exit 1
fi

build_type="$1"
build_path="$2"
source_path="$3"
echo "bitten build.sh type=${build_type} src=${sourcepath} bld=${build_path}"

echo "creating build directory..."

mkdir -p "${build_dir}" || exit 1
pushd "${build_dir}"
cmake "${source_dir}" -DCMAKE_BUILD_TYPE=${build_type} -DENABLE_MONITOR_GUI=No || exit 1
make -s all || exit 1
ctest
popd
