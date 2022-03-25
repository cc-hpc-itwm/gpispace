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

macro (_swh_deployment_gpispace)
endmacro()
macro (_swh_deployment_implementation_module)
endmacro()
macro (_swh_deployment_implementation_exe)
endmacro()

# Append any directories outside the project that are in the linker
# search path or contain linked library files to the rpath of all
# installed binaries. Additionally, add the "implicit link
# directories" as well, which is platform and compiler dependent
# paths. This allows the execution environment to not have
# LD_LIBRARY_PATH set but still point to the dependencies found at
# configure time.
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
set (CMAKE_CXX_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH true)

# Use RPATH instead of RUNPATH to avoid LD_LIBRARY_PATH overrides.
add_link_options (LINKER:--disable-new-dtags)
