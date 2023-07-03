# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard ()

include (${CMAKE_CURRENT_LIST_DIR}/bootstrap-macros.cmake)

#############################################################################
# Bootstrap Package Locations
#############################################################################

bootstrap_package (
  util-cmake
  URL http://gitlab.hpc.devnet.itwm.fhg.de/shared/cmake/-/archive/master/cmake-master.tar.gz
  VERSION 1.0.0
  VERSION_COMPATIBILITY ExactVersion
)
bootstrap_package (
  util-generic
  URL http://gitlab.hpc.devnet.itwm.fhg.de/shared/util-generic/-/archive/master/util-generic-master.tar.gz
  VERSION 1.0.0
  VERSION_COMPATIBILITY ExactVersion
)
