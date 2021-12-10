# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
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

###############################################################################
# Includes
###############################################################################

include (util-cmake/add_system_test)

###############################################################################
# Definitions
###############################################################################

# the notation $ENV\{<var-name>} is necessary to delay the variable
# evaluation until the system test is run
set (_gspc_testing_arguments
  "--gspc-home=${CMAKE_INSTALL_PREFIX}"
  "--nodefile=$ENV\{GSPC_NODEFILE_FOR_TESTS}"
  "--rif-strategy=${TESTING_RIF_STRATEGY}"
)

###############################################################################
# System Tests
###############################################################################

util_cmake_add_system_test (aggregate_sum
  CMAKE_DEFINITIONS
    -D GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}
  ARGS
    ${_gspc_testing_arguments}
    --topology=worker:2
    --N=100
)
