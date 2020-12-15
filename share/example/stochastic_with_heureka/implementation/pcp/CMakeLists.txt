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

extended_add_library (NAME pcp
  TYPE SHARED
  SOURCES "pcp.cpp"
  LIBRARIES GPISpace::workflow_development
  INSTALL INSTALL_DESTINATION "implementation"
)
bundle_GPISpace_add_rpath (TARGET pcp
  INSTALL_DIRECTORY "implementation"
)

extended_add_executable (NAME run-pcp
  SOURCES "run.cpp"
  LIBRARIES run-stochastic_with_heureka pcp
  DONT_APPEND_EXE_SUFFIX
  INSTALL
)
bundle_GPISpace_add_rpath (TARGET run-pcp
  INSTALL_DIRECTORY "bin"
)
