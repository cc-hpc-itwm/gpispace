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

set (_cmake_minimum_required_version "3.15")

if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  if (CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS _cmake_minimum_required_version)
    message (FATAL_ERROR
      "The minimum required CMake version of the including project is too low! \
      Projects using util-cmake are required to set CMake version \
      ${_cmake_minimum_required_version} or above as their minimum required version! \
      \nA line of code such as \
      \
      \n    cmake_minimum_required(VERSION ${_cmake_minimum_required_version})\
      \
      \nis required to be added at the top of the including project's root CMakeLists.txt."
    )
  endif ()
else ()
  cmake_minimum_required (VERSION ${_cmake_minimum_required_version})
endif ()

project (
  util-cmake
  VERSION 1.0.0
)

###############################################################################
# Set module paths
###############################################################################

list (APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/modules"
  "${PROJECT_SOURCE_DIR}/include"
)

###############################################################################
# Includes
###############################################################################

include (GNUInstallDirs)
include (CMakePackageConfigHelpers)

include (util-cmake/projects)

###############################################################################
# Meta information
###############################################################################

util_cmake_is_project_root (_project_is_root)
util_cmake_project_entry_name (_project_entry)

###############################################################################
# Options
###############################################################################

option (${_project_entry}_INSTALL
  "Install ${PROJECT_NAME}"
  ${_project_is_root}
)
mark_as_advanced (${_project_entry}_INSTALL)

set (${_project_entry}_CONFIG_INSTALL_DIR
  "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake"
  CACHE PATH
  "${PROJECT_NAME} package configuration and cmake scripts installation path prefix. \
  (default: ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)"
)
mark_as_advanced (${_project_entry}_CONFIG_INSTALL_DIR)
set (_project_config_install_dir ${${_project_entry}_CONFIG_INSTALL_DIR})

###############################################################################
# Project operations
###############################################################################

if (NOT _project_is_root)
  set (CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH}"
    PARENT_SCOPE
  )
endif ()

###############################################################################
# Install and configure package find script
###############################################################################

if (${_project_entry}_INSTALL)
  install (
    DIRECTORY
      "${PROJECT_SOURCE_DIR}/modules"
      "${PROJECT_SOURCE_DIR}/include"
    DESTINATION
      ${_project_config_install_dir}
    USE_SOURCE_PERMISSIONS
  )

  configure_package_config_file (
    ${PROJECT_NAME}-config.cmake.in
    ${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${_project_config_install_dir}
    PATH_VARS
      _project_config_install_dir
  )

  write_basic_package_version_file (
    ${PROJECT_NAME}-config-version.cmake
    COMPATIBILITY SameMajorVersion
  )

  install (
    FILES
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION
      ${_project_config_install_dir}
  )
endif ()
