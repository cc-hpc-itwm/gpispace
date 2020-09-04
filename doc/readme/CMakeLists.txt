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

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/getting_started.md"
  DESTINATION "share/gspc/"
)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/GPISpace_arch.png"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/gpispace_example_pnet.png"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/petri_net.png"
  DESTINATION "share/gspc/img/"
)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/compute_and_aggregate.xpnet"
               "${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp"
  DESTINATION "share/gspc/example-compute_and_aggregate/"
)
install (PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/build.sh"
  DESTINATION "share/gspc/example-compute_and_aggregate/"
)

# \todo `fhg_add_test()` requires SOURCES. Factor out the common part...
add_test (NAME doc-readme-getting_started-build
  COMMAND "${CMAKE_INSTALL_PREFIX}/share/gspc/example-compute_and_aggregate/build.sh"
          "${CMAKE_INSTALL_PREFIX}"
          "${CMAKE_CURRENT_BINARY_DIR}/doc-readme-getting_started-build"
          "" # don't start monitor
          "${CMAKE_CXX_COMPILER}"
)
set_property (TEST doc-readme-getting_started-build
  APPEND PROPERTY LABELS "requires_installation"
)
