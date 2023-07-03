# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include (${CMAKE_CURRENT_LIST_DIR}/compiler/gcc.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/tool/ccache.cmake)

set (CMAKE_C_FLAGS_INIT -Werror)
set (CMAKE_CXX_FLAGS_INIT -Werror)
