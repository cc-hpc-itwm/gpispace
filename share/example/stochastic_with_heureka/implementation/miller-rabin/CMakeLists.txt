# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

option (BUILD_MILLER_RABIN "Build the miller-rabin prime test example
  implementation. Requires GMP." ON
)

set (GMP_PREFIX "" CACHE PATH "The path to the prefix of a GMP installation")
if ("${GMP_PREFIX}" STREQUAL "" AND NOT "$ENV{GMP_PREFIX}" STREQUAL "")
  set (GMP_PREFIX "$ENV{GMP_PREFIX}")
endif()

find_path (GMP_INCLUDE_DIR gmpxx.h
  PATHS ${GMP_PREFIX}/include
)
find_library (GMP_LIBRARY_gmp NAMES gmp
  PATHS ${GMP_PREFIX}/lib
)
find_library (GMP_LIBRARY_gmpxx NAMES gmpxx
  PATHS ${GMP_PREFIX}/lib
)

if (NOT (GMP_INCLUDE_DIR AND GMP_LIBRARY_gmp AND GMP_LIBRARY_gmpxx))
  message (WARNING "Could not find GMP, skipping miller-rabin implementation."
    "(GMP_INCLUDE_DIR=${GMP_INCLUDE_DIR}, GMP_LIBRARY_gmp=${GMP_LIBRARY_gmp}, "
    "GMP_LIBRARY_gmpxx=${GMP_LIBRARY_gmpxx})"
  )
  set (BUILD_MILLER_RABIN OFF)
endif()

if (BUILD_MILLER_RABIN)
  extended_add_library (NAME gmp
    LIBRARIES "${GMP_LIBRARY_gmpxx}"
              "${GMP_LIBRARY_gmp}"
    INCLUDE_DIRECTORIES INTERFACE "${GMP_INCLUDE_DIR}"
  )

  extended_add_library (NAME util
    TYPE STATIC
    SOURCES "util.cpp"
    LIBRARIES gmp
    POSITION_INDEPENDENT
  )

  add_unit_test (NAME miller-rabin-utils
    SOURCES "util-test.cpp"
    LIBRARIES util
    USE_BOOST
  )

  extended_add_library (NAME miller-rabin
    TYPE MODULE
    SOURCES "miller-rabin.cpp"
    LIBRARIES util
              GPISpace::workflow_development
              gmp
    INSTALL INSTALL_DESTINATION "implementation"
  )
  bundle_GPISpace_add_rpath (TARGET miller-rabin
    INSTALL_DIRECTORY "implementation"
  )

  extended_add_executable (NAME run-miller-rabin
    SOURCES "run.cpp"
    DONT_APPEND_EXE_SUFFIX
    LIBRARIES run-stochastic_with_heureka
              gmp
    INSTALL
  )
  bundle_GPISpace_add_rpath (TARGET run-miller-rabin
    INSTALL_DIRECTORY "bin"
  )
endif()

add_subdirectory (user-example)
