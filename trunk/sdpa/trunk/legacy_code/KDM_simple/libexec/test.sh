#!/bin/sh

pushd /p/hpc/sdpa/ap/git/KDM_VM

./libexec/test_load_module ./libexec/libfvm-pc.so ./build.gcc/libkdm.so

popd
