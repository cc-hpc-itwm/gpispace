# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

find_program (CCACHE_EXECUTABLE
  NAMES
    ccache
    ccache-swig
)

if (CCACHE_EXECUTABLE)
  set (CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
  set (CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
endif()
