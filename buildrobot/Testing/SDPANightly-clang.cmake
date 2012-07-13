# -*- mode: cmake; -*-
set(EXTERNAL_SOFTWARE "/home/projects/sdpa/external_software/")
set(CLANG_BIN "${EXTERNAL_SOFTWARE}/clang/2.9/bin")
set(ENV{CC} "${CLANG_BIN}/clang")
set(ENV{CXX} "${CLANG_BIN}/clang++")
set(COMPILER_ID "clang")
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/SDPANightly.cmake")
