cmake_minimum_required (VERSION 3.16)

project (iml
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
  find_package (util-generic 1.0.0 EXACT REQUIRED)
  find_package (util-rpc 1.0.0 EXACT REQUIRED)
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

include (util-cmake/parse_arguments)
include (util-cmake/projects)

include (iml/add_macros)

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

if (${_project_entry}_BUILD_TESTING)
  set (IML_TESTING_RIF_STRATEGY
    "ssh"
    CACHE STRING
    "The strategy to bootstrap passed to --iml-rif-strategy in tests."
  )

  set (IML_TESTING_RIF_STRATEGY_PARAMETERS
    ""
    CACHE STRING
    "A semicolon-separated list of parameters to pass to
    --iml-rif-strategy-parameters in tests."
  )

  set (IML_TESTING_BEEGFS_DIRECTORY
    ""
    CACHE PATH
    "(Shared) directory to use for BeeGFS tests."
  )
endif()

###############################################################################
# Definitions
###############################################################################

set (iml_devel_files_in_installation
  "include/iml/AllocationHandle.hpp"
  "include/iml/AllocationHandle.ipp"
  "include/iml/Client.hpp"
  "include/iml/MemcpyID.hpp"
  "include/iml/MemoryLocation.hpp"
  "include/iml/MemoryLocation.ipp"
  "include/iml/MemoryOffset.hpp"
  "include/iml/MemoryRegion.hpp"
  "include/iml/MemoryRegion.ipp"
  "include/iml/MemorySize.hpp"
  "include/iml/Rifs.hpp"
  "include/iml/RuntimeSystem.hpp"
  "include/iml/SegmentAndAllocation.hpp"
  "include/iml/SegmentDescription.hpp"
  "include/iml/SegmentHandle.hpp"
  "include/iml/SegmentHandle.ipp"
  "include/iml/SharedMemoryAllocation.hpp"
  "include/iml/SharedMemoryAllocationHandle.hpp"
  "include/iml/SharedMemoryAllocationHandle.ipp"
  "include/iml/beegfs/SegmentDescription.hpp"
  "include/iml/beegfs/SegmentDescription.ipp"
  "include/iml/detail/dllexport.hpp"
  "include/iml/detail/dllexport.ipp"
  "include/iml/gaspi/NetdevID.hpp"
  "include/iml/gaspi/NetdevID.ipp"
  "include/iml/gaspi/SegmentDescription.hpp"
  "include/iml/gaspi/SegmentDescription.ipp"
  "include/iml/rif/EntryPoint.hpp"
  "include/iml/rif/EntryPoint.ipp"
  "include/iml/rif/EntryPoints.hpp"
  "include/iml/rif/bootstrap.hpp"
  "include/iml/rif/strategies.hpp"
  "include/iml/rif/teardown.hpp"
)

set (iml_files_in_installation
  "bin/iml-bootstrap-rifd"
  "bin/iml-teardown-rifd"
  "lib/libIML-Client.so"
  "libexec/iml/iml-gpi-server"
  "libexec/iml/iml-rifd"
  "libexec/iml/libIMLPrivate-Installation.so"
  "${_project_config_install_dir}/iml-config.cmake"
  "${_project_config_install_dir}/iml-config-version.cmake"
  ${iml_devel_files_in_installation}
)

###############################################################################
# Find dependencies
###############################################################################

find_package (Boost 1.61 REQUIRED
  COMPONENTS
    filesystem
    iostreams
    program_options
    serialization
    system
    thread
)
# BeeGFS: "found" via ${PROJECT_SOURCE_DIR}/external/
find_package (GASPI 1.5 REQUIRED)
find_package (Libssh2 REQUIRED)
find_package (OpenSSL REQUIRED)

###############################################################################
# Project targets and tests
###############################################################################

extended_add_library (NAME Headers
  NAMESPACE IML
  INCLUDE_DIRECTORIES INTERFACE $<BUILD_INTERFACE: ${PROJECT_SOURCE_DIR}/include>
                      INTERFACE $<INSTALL_INTERFACE: ${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features (IML-Headers
  INTERFACE "${_project_cxx_std}"
)

add_subdirectory (external)
add_subdirectory (src/iml)

if (${_project_entry}_BUILD_TESTING)
  add_subdirectory (test/iml)
endif()

include (${PROJECT_SOURCE_DIR}/meta/meta.cmake OPTIONAL)

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
