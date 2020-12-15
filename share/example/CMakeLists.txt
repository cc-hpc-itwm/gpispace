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

set (example_install_prefix "share/gspc/example")

add_subdirectory (accumulate_values_from_multiple_submissions/test)
add_subdirectory (concurrent)
add_subdirectory (map/test/log)
add_subdirectory (map/test/transform_file)
add_subdirectory (n_of_m/test)
add_subdirectory (pi/test)
add_subdirectory (ping-pong/test)
add_subdirectory (priorities/test)
add_subdirectory (split_join)
add_subdirectory (stream)
add_subdirectory (subnetwork)
add_subdirectory (use_sequence/test)
add_subdirectory (vmem_1_to_n/test)
add_subdirectory (workerlist)

install (DIRECTORY "stochastic_with_heureka"
  DESTINATION "${example_install_prefix}"
  USE_SOURCE_PERMISSIONS
)
# \note To avoid having the cmake submodule twice and possible out of
# sync, just copy the top level one into the example
# installation. This implies that a non-installed
# stochastic_with_heureka does *not* build unless a cmake submodule is
# manually checked out/copied there.
install (DIRECTORY "${CMAKE_SOURCE_DIR}/cmake"
  DESTINATION "${example_install_prefix}/stochastic_with_heureka"
  USE_SOURCE_PERMISSIONS
)

set (installed_stochastic_with_heureka_path
  "${CMAKE_INSTALL_PREFIX}/${example_install_prefix}/stochastic_with_heureka"
)
add_test (NAME example-stochastic_with_heureka
  COMMAND "${installed_stochastic_with_heureka_path}/selftest"
)
set_tests_properties (example-stochastic_with_heureka PROPERTIES
  WORKING_DIRECTORY "${installed_stochastic_with_heureka_path}"
  ENVIRONMENT "CC=${CMAKE_CC_COMPILER};CXX=${CMAKE_CXX_COMPILER};GSPC_HOME=${CMAKE_INSTALL_PREFIX}"
  LABELS "requires_installation"
  REQUIRED_FILES "${FILES_REQUIRED_IN_INSTALLATION}"
)
