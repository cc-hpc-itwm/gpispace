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

add_library (asianopt-base OBJECT "asianopt.cpp")
set_property (TARGET asianopt-base PROPERTY POSITION_INDEPENDENT_CODE ON)

swh_add_implementation_module (NAME asianopt
  SOURCES "asianopt-wrapper.cpp"
  $<TARGET_OBJECTS:asianopt-base>
)

swh_add_implementation_exe (NAME asianopt
  SOURCES "asianopt-run.cpp"
  $<TARGET_OBJECTS:asianopt-base>
)

add_executable (asianopt-runs.test
  "test/asianopt.cpp" $<TARGET_OBJECTS:asianopt-base>
)
target_compile_definitions (asianopt-runs.test PRIVATE
  "-DBOOST_TEST_MODULE=\"asianopt-runs\""
)
target_link_libraries (asianopt-runs.test PRIVATE Boost::unit_test_framework)
add_test (NAME asianopt-runs COMMAND $<TARGET_FILE:asianopt-runs.test>)


swh_add_implementation_module (NAME barropt
  SOURCES "barropt.cpp"
)
swh_add_implementation_exe (NAME barropt
  SOURCES "barropt-run.cpp"
)
