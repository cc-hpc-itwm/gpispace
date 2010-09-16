#!/bin/sh
# configures and builds a specific build

if [ $# -ne 5 ]; then
  echo "usage: $0 <type> <path-src> <path-build> <revision> <build>"
  exit 1
fi

build_type="$1"
source_path="$(pwd)/$2"
build_path="$(pwd)/$3"
revision=$4
build=$5
echo "bitten build.sh type=${build_type} src=${source_path} bld=${build_path}"

echo "creating build directory..."

mkdir -p "${build_path}" || exit 1
pushd "${build_path}"
cmake "${source_path}" \
  -DCMAKE_BUILD_TYPE=${build_type} \
  -DENABLE_MONITOR_GUI=No \
  -DBOOST_ROOT=/opt/boost \
  -DPROJECT_REVISION="${revision}" \
  -DPROJECT_BUILD="${build}" \
  || exit 1
make -s all || exit 1
ctest
popd
