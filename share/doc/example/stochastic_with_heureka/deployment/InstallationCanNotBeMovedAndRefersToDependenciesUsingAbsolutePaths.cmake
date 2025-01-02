# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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
