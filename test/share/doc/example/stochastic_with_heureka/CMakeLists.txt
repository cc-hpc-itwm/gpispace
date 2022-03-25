# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

list (TRANSFORM
  TESTING_RIF_STRATEGY_PARAMETERS
  PREPEND
    "--rif-strategy-parameters="
  OUTPUT_VARIABLE
    _testing_rif_strategy_parameters_with_arg_prefix
)

add_test (
  NAME
    doc_example_stochastic_with_heureka
  COMMAND
    selftest
    ${CMAKE_COMMAND}
    "${TESTING_RIF_STRATEGY}"
    ${_testing_rif_strategy_parameters_with_arg_prefix}
  WORKING_DIRECTORY
    ${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/doc/example/stochastic_with_heureka
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY ENVIRONMENT
    "CXX=${CMAKE_CXX_COMPILER}"
    "GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}"
    "SWH_INSTALL_DIR=${SHARED_DIRECTORY_FOR_TESTS}/stochastic_with_heureka"
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY LABELS
    "requires_installation"
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY REQUIRED_FILES
    "${FILES_REQUIRED_IN_INSTALLATION}"
)
