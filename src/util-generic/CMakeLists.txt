cmake_minimum_required (VERSION 3.15 FATAL_ERROR)

project (util-generic
  VERSION 1.0.0
  LANGUAGES CXX
)

###############################################################################
# Bootstrap packages
###############################################################################

# temporary compatibility fix
# if the project is not root skip bootstrapping
# NOTE: the find_package call can move to the find dependencies section later on
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  include (${PROJECT_SOURCE_DIR}/bootstrap/packages.cmake)
  find_package (util-cmake 1.0.0 EXACT REQUIRED)
endif ()

###############################################################################
# Set module paths
###############################################################################

list (APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
)

###############################################################################
# Includes
###############################################################################

include (CMakePackageConfigHelpers)
include (CTest)
include (GNUInstallDirs)

include (util-generic/build_checks)

include (util-cmake/add_macros)
include (util-cmake/beautify_find_boost)
include (util-cmake/projects)

###############################################################################
# Meta information
###############################################################################

util_cmake_is_project_root (_project_is_root)
util_cmake_project_entry_name (_project_entry)

###############################################################################
# Options
###############################################################################

if (NOT DEFINED ${_project_entry}_BUILD_TESTING)
  if (BUILD_TESTING)
    set (${_project_entry}_BUILD_TESTING ${BUILD_TESTING} CACHE BOOL "Build ${PROJECT_NAME} tests")
  else ()
    set (${_project_entry}_BUILD_TESTING OFF CACHE BOOL "Build ${PROJECT_NAME} tests")
  endif ()
endif ()

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

set (${_project_entry}_CXX_STANDARD
  14
  CACHE STRING
  "${PROJECT_NAME} C++ standard"
)
mark_as_advanced (${_project_entry}_CXX_STANDARD)
set (_project_cxx_std "cxx_std_${${_project_entry}_CXX_STANDARD}")

###############################################################################
# Find dependencies
###############################################################################

find_package (Threads REQUIRED)

find_boost (1.61 REQUIRED
  COMPONENTS filesystem
             program_options
             serialization
             system
             thread
             unit_test_framework
)

###############################################################################
# Project targets and tests
###############################################################################

extended_add_library (NAME Generic-Headers
  NAMESPACE Util
  INCLUDE_DIRECTORIES INTERFACE $<BUILD_INTERFACE: ${PROJECT_SOURCE_DIR}/include>
                      INTERFACE $<INSTALL_INTERFACE: ${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES
    INTERFACE
      Boost::base
      Boost::serialization
      Boost::filesystem
      Boost::unit_test_framework
      Boost::system
)
target_compile_features (Util-Generic-Headers
  INTERFACE "${_project_cxx_std}"
)

add_subdirectory (src/util-generic)
add_subdirectory (test/util-generic)

###############################################################################
# Install and configure package find script
###############################################################################

if (${_project_entry}_INSTALL)
  install (
    DIRECTORY
      "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION
      "${CMAKE_INSTALL_INCLUDEDIR}"
  )

  set (_origin ".")
  configure_package_config_file (
    ${PROJECT_NAME}-config.cmake.in
    ${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${_project_config_install_dir}
    PATH_VARS
      _project_config_install_dir
      _origin
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
