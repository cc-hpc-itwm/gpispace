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

include (util-cmake/add_macros)
include (util-cmake/beautify_find_boost)

# Dependencies

find_boost (1.61 REQUIRED COMPONENTS
  filesystem
  iostreams
  program_options
  serialization
  system
  thread
)

# Util::Generic: "found" via ${PROJECT_SOURCE_DIR}/src/ and
# RPC: "found" via ${PROJECT_SOURCE_DIR}/src/

# BeeGFS: "found" via ${PROJECT_SOURCE_DIR}/external/
find_package (GASPI 1.3.1 REQUIRED)

find_package (Libssh2 REQUIRED)
find_package (OpenSSL REQUIRED)


# Configuration

set (IML_TESTING_RIF_STRATEGY "ssh"
  CACHE STRING
  "The strategy to bootstrap passed to --iml-rif-strategy in tests."
)
set (IML_TESTING_RIF_STRATEGY_PARAMETERS ""
  CACHE STRING
  "A semicolon-separated list of parameters to pass to
   --iml-rif-strategy-parameters in tests."
)
set (IML_TESTING_BEEGFS_DIRECTORY ""
  CACHE PATH
  "(Shared) directory to use for BeeGFS tests."
)


# Targets

#! \returns List _rpaths relative to install destination.
macro (_iml_assemble_rpath _install_destination _rpath)
  # \todo Add RPATHs automatically by tracking INSTALL_DESTINATION of
  # linked libraries.
  set (_rpaths "")
  string (REGEX REPLACE "[^/]+" ".." _rpath_mid "${_install_destination}")
  foreach (_elem IN ITEMS ${_rpath})
    list (APPEND _rpaths "\$ORIGIN/${_rpath_mid}/${_elem}")
  endforeach()
endmacro()

#! Wrapper for shared/cmake's \see extended_add_library() which
#! additionally
#! - Requires \a NAMESPACE.
#! - Lets `RPATH` be relative paths.
#! - Adds `--exclude-libs,ALL` to link options so that resulting
#!   libraries do not re-export unintended symbols.
#! - Always adds `CREATE_BUNDLE_INFO` on \a INSTALL.
function (iml_add_library)
  set (options INSTALL)
  set (one_value_options INSTALL_DESTINATION)
  set (multi_value_options RPATH)
  set (required_options)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (_arg_INSTALL)
    _iml_assemble_rpath ("${_arg_INSTALL_DESTINATION}" "${_arg_RPATH}")

    extended_add_library (${_arg_UNPARSED_ARGUMENTS}
      INSTALL
        INSTALL_DESTINATION "${_arg_INSTALL_DESTINATION}"
        CREATE_BUNDLE_INFO
        RPATH ${_rpaths}
    )
  else()
    extended_add_library (${_arg_UNPARSED_ARGUMENTS})
  endif()
endfunction()

#! Wrapper for shared/cmake's \see extended_add_executable() which
#! additionally
#! - Lets `RPATH` be relative paths.
#! - Adds `--exclude-libs,ALL` to link options so that resulting
#!   libraries do not re-export unintended symbols.
#! - Always adds `CREATE_BUNDLE_INFO` on \a INSTALL.
#! - Always adds `DONT_APPEND_EXE_SUFFIX` and `NO_RDYNAMIC`.
function (iml_add_executable)
  set (options INSTALL)
  set (one_value_options INSTALL_DESTINATION)
  set (multi_value_options RPATH)
  set (required_options)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_fwd_args
    ${_arg_UNPARSED_ARGUMENTS}
    DONT_APPEND_EXE_SUFFIX
    NO_RDYNAMIC
  )

  if (_arg_INSTALL)
    _iml_assemble_rpath ("${_arg_INSTALL_DESTINATION}" "${_arg_RPATH}")

    extended_add_executable (${_fwd_args}
      INSTALL
        INSTALL_DESTINATION "${_arg_INSTALL_DESTINATION}"
        CREATE_BUNDLE_INFO
        RPATH ${_rpaths}
    )
  else()
    extended_add_executable (${_fwd_args})
  endif()
endfunction()

iml_add_library (NAME Shared
  NAMESPACE IMLPrivate
  SOURCES "vmem/gaspi/pc/segment/segment.cpp"
  LIBRARIES
    PRIVATE Util::Generic
    PUBLIC Boost::base
    PRIVATE Boost::system
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  POSITION_INDEPENDENT
)

iml_add_library (NAME Client
  NAMESPACE IML
  TYPE SHARED
  VISIBILITY_HIDDEN
  SOURCES # API files
          "AllocationHandle.cpp"
          "Client.cpp"
          "MemoryLocation.cpp"
          "MemoryRegion.cpp"
          "Rifs.cpp"
          "RuntimeSystem.cpp"
          "SegmentAndAllocation.cpp"
          "SegmentHandle.cpp"
          "SharedMemoryAllocation.cpp"
          "SharedMemoryAllocationHandle.cpp"
          "beegfs/SegmentDescription.cpp"
          "gaspi/NetdevID.cpp"
          "gaspi/SegmentDescription.cpp"
          "rif/EntryPoint.cpp"
          "rif/EntryPoints.cpp"
          "rif/bootstrap.cpp"
          "rif/strategies.cpp"
          "rif/teardown.cpp"

          # non-API files
          "detail/startup_and_shutdown.cpp"
          "rif/strategy/local.cpp"
          "rif/strategy/meta.cpp"
          "rif/strategy/pbsdsh.cpp"
          "rif/strategy/ssh.cpp"
          "rif/strategy/ssh/context.cpp"
          "rif/strategy/ssh/detail.cpp"
          "rif/strategy/ssh/session.cpp"
          "rif/strategy/system_with_blocked_SIGCHLD.cpp"
  LIBRARIES
    PRIVATE IMLPrivate::Installation
    PRIVATE IMLPrivate::Shared
    PRIVATE Util::RPC
    PRIVATE Util::Generic
    PRIVATE Libssh2::libssh2
    PRIVATE OpenSSL::Crypto
    PRIVATE OpenSSL::SSL
    PUBLIC Boost::filesystem
    PUBLIC Boost::program_options
    PUBLIC Boost::serialization
    PRIVATE Boost::thread
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  INSTALL
    INSTALL_DESTINATION "lib"
    RPATH "libexec/iml"
)

iml_add_library (NAME Server
  NAMESPACE IMLPrivate
  SOURCES "vmem/dtmmgr.cpp"
          "vmem/gaspi/gpi/gaspi.cpp"
          "vmem/gaspi/pc/container/manager.cpp"
          "vmem/gaspi/pc/global/topology.cpp"
          "vmem/gaspi/pc/memory/beegfs_area.cpp"
          "vmem/gaspi/pc/memory/gaspi_area.cpp"
          "vmem/gaspi/pc/memory/handle_generator.cpp"
          "vmem/gaspi/pc/memory/manager.cpp"
          "vmem/gaspi/pc/memory/memory_area.cpp"
          "vmem/gaspi/pc/memory/shm_area.cpp"
          "vmem/gaspi_context.cpp"
          "vmem/segment/beegfs.cpp"
          "vmem/tmmgr.cpp"
  LIBRARIES
    PUBLIC IML::Client
    PUBLIC IMLPrivate::Shared
    PUBLIC Util::RPC
    PRIVATE Util::Generic
    PRIVATE BeeGFS::ClientDevel
    PRIVATE GASPI::static
    PUBLIC Boost::filesystem
    PUBLIC Boost::serialization
    PRIVATE Boost::system
    PUBLIC Boost::thread
)

if (BUILD_TESTING)
  iml_add_library (NAME Testing
    NAMESPACE IMLPrivate
    SOURCES "testing/beegfs_directory.cpp"
            "testing/parse_command_line.cpp"
            "testing/random/AllocationHandle.cpp"
            "testing/random/MemoryLocation.cpp"
            "testing/random/MemoryRegion.cpp"
            "testing/random/SegmentHandle.cpp"
            "testing/set_nodefile_from_environment.cpp"
            "testing/virtual_memory_socket_name_for_localhost.cpp"
    LIBRARIES
      PUBLIC IML::Client
      PUBLIC Util::Generic
      PUBLIC Boost::program_options
      PUBLIC Boost::filesystem
  )
endif()

iml_add_executable (NAME iml-gpi-server
  SOURCES "server/iml-gpi-server.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::rif-started_process_promise
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
    RPATH "lib"
)

iml_add_library (NAME Installation
  NAMESPACE IMLPrivate
  TYPE SHARED
  VISIBILITY_HIDDEN
  SOURCES "detail/Installation.cpp"
  LIBRARIES
    PRIVATE Util::Generic
    PRIVATE Boost::filesystem
  COMPILE_DEFINITIONS
# The number of /.. to append to the path of the installed libary to
# get back up to the installation root:
#   install/libexec/iml/libIMLPrivate-Installation.so/../../../ = install/
    PRIVATE "-DIML_INSTALLATION_SENTINEL_SUBDIR_COUNT=3"
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
)

iml_add_executable (NAME iml-rifd
  SOURCES "rif/iml-rifd.cpp"
          "rif/execute_and_get_startup_messages.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Installation
            Util::RPC
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
    RPATH "lib"
          "libexec/iml"
)

iml_add_library (NAME rif-started_process_promise
  NAMESPACE IMLPrivate
  SOURCES "rif/started_process_promise.cpp"
  LIBRARIES
    PRIVATE Util::Generic
    PRIVATE Boost::iostreams
    PRIVATE Boost::serialization
)

iml_add_executable (NAME iml-bootstrap-rifd
  SOURCES "rif/iml-bootstrap-rifd.cpp"
  LIBRARIES IML::Client
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  INSTALL
    INSTALL_DESTINATION "bin"
    RPATH "lib"
)
iml_add_executable (NAME iml-teardown-rifd
  SOURCES "rif/iml-teardown-rifd.cpp"
  LIBRARIES IML::Client
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  INSTALL
    INSTALL_DESTINATION "bin"
    RPATH "lib"
)


# Installation

set (iml_files_in_installation
  "bin/iml-bootstrap-rifd"
  "bin/iml-teardown-rifd"
  "lib/libIML-Client.so"
  "libexec/iml/iml-gpi-server"
  "libexec/iml/iml-rifd"
  "libexec/iml/libIMLPrivate-Installation.so"
)

install (FILES
  ${PROJECT_SOURCE_DIR}/include/iml/AllocationHandle.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/AllocationHandle.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/Client.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/MemcpyID.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/MemoryLocation.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/MemoryLocation.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/MemoryOffset.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/MemoryRegion.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/MemoryRegion.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/MemorySize.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/Rifs.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/RuntimeSystem.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SegmentAndAllocation.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SegmentDescription.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SegmentHandle.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SegmentHandle.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/SharedMemoryAllocation.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SharedMemoryAllocationHandle.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/SharedMemoryAllocationHandle.ipp
  DESTINATION include/iml/
)

list (APPEND iml_devel_files_in_installation
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
)

install (FILES
  ${PROJECT_SOURCE_DIR}/include/iml/beegfs/SegmentDescription.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/beegfs/SegmentDescription.ipp
  DESTINATION include/iml/beegfs/
)

list (APPEND iml_devel_files_in_installation
  "include/iml/beegfs/SegmentDescription.hpp"
  "include/iml/beegfs/SegmentDescription.ipp"
)

install (FILES
  ${PROJECT_SOURCE_DIR}/include/iml/detail/dllexport.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/detail/dllexport.ipp
  DESTINATION include/iml/detail/
)

list (APPEND iml_devel_files_in_installation
  "include/iml/detail/dllexport.hpp"
  "include/iml/detail/dllexport.ipp"
)

install (FILES
  ${PROJECT_SOURCE_DIR}/include/iml/gaspi/NetdevID.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/gaspi/NetdevID.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/gaspi/SegmentDescription.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/gaspi/SegmentDescription.ipp
  DESTINATION include/iml/gaspi/
)

list (APPEND iml_devel_files_in_installation
  "include/iml/gaspi/NetdevID.hpp"
  "include/iml/gaspi/NetdevID.ipp"
  "include/iml/gaspi/SegmentDescription.hpp"
  "include/iml/gaspi/SegmentDescription.ipp"
)

install (FILES
  ${PROJECT_SOURCE_DIR}/include/iml/rif/EntryPoint.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/rif/EntryPoint.ipp
  ${PROJECT_SOURCE_DIR}/include/iml/rif/EntryPoints.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/rif/bootstrap.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/rif/strategies.hpp
  ${PROJECT_SOURCE_DIR}/include/iml/rif/teardown.hpp
  DESTINATION include/iml/rif/
)

list (APPEND iml_devel_files_in_installation
  "include/iml/rif/EntryPoint.hpp"
  "include/iml/rif/EntryPoint.ipp"
  "include/iml/rif/EntryPoints.hpp"
  "include/iml/rif/bootstrap.hpp"
  "include/iml/rif/strategies.hpp"
  "include/iml/rif/teardown.hpp"
)

set (IML_cmakeconfig_destination "share/IML/cmake")

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/IMLConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/IMLConfigVersion.cmake"
  DESTINATION "${IML_cmakeconfig_destination}"
)

list (APPEND iml_devel_files_in_installation
  "${IML_cmakeconfig_destination}/IMLConfig.cmake"
  "${IML_cmakeconfig_destination}/IMLConfigVersion.cmake"
)

list (APPEND iml_files_in_installation ${iml_devel_files_in_installation})

## CMake package config

set (IML_install_root ".")
set (IML_include_dir "include")
set (IML_libIMLClient_path "lib/libIML-Client.so")
set (IML_libexec_dir "libexec/iml")

include (CMakePackageConfigHelpers)
configure_package_config_file (IMLConfig.cmake.in IMLConfig.cmake
  INSTALL_DESTINATION "${IML_cmakeconfig_destination}"
  PATH_VARS IML_include_dir
            IML_install_root
            IML_libIMLClient_path
            IML_libexec_dir
)
write_basic_package_version_file (IMLConfigVersion.cmake
  COMPATIBILITY ExactVersion
)


# Tests

set (_iml_testing_port_counter 11520
  CACHE INTERNAL
  "Counter for unique --iml-vmem-port arguments."
)
set (_iml_testing_port_per_test 100)

set (_iml_testing_beegfs_tests)

## Unit tests

#! Wrapper for shared/cmake's \see add_unit_test() which additionally
#! - Takes \a NEEDS_BEEGFS to only run the test if the global variable
#!   \c IML_TESTING_BEEGFS_DIRECTORY is on a valid BeeGFS mountpoint.
#! - Takes \a NEEDS_INSTALLATION to automatically override the
#!   installation sentinel derived path. Does **not** add
#!   `REQUIRED_FILES`, \see iml_add_system_test() instead.
function (iml_add_unit_test)
  if (NOT BUILD_TESTING)
    return()
  endif()

  set (options NEEDS_BEEGFS NEEDS_INSTALLATION)
  set (one_value_options NAME)
  set (multi_value_options ARGS)
  set (required_options)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_test_name "iml-${_arg_NAME}")

  if (_arg_NEEDS_BEEGFS)
    list (INSERT _arg_ARGS 0
      --beegfs-directory "${IML_TESTING_BEEGFS_DIRECTORY}"
    )
    list (APPEND _iml_testing_beegfs_tests "${_test_name}")
    set (_iml_testing_beegfs_tests ${_iml_testing_beegfs_tests} PARENT_SCOPE)
  endif()

  add_unit_test (NAME "${_test_name}"
    USE_BOOST
    ARGS ${_arg_ARGS}
    ${_arg_UNPARSED_ARGUMENTS}
  )

  # When unit tests use our installation, they still see the
  # uninstalled installation sentinel. As our build directory
  # structure differs from the install directory binaries would not be
  # found. Thus, override with the actual installation directory.
  if (_arg_NEEDS_INSTALLATION)
    set_tests_properties (${_test_name} PROPERTIES ENVIRONMENT
      "IML_TESTING_OVERRIDE_INSTALLATION_PREFIX=${CMAKE_INSTALL_PREFIX}"
    )
  endif()
endfunction()

iml_add_unit_test (NAME handle_generator
  SOURCES "vmem/gaspi/tests/handle_generator.cpp"
  LIBRARIES IMLPrivate::Server
)

iml_add_unit_test (NAME memory_area
  SOURCES "vmem/gaspi/tests/memory_area.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME segment
  SOURCES "vmem/gaspi/tests/segment.cpp"
  LIBRARIES IMLPrivate::Server
)

iml_add_unit_test (NAME api
  SOURCES "tests/Client.cpp"
  LIBRARIES IML::Client
            Util::Generic
)

iml_add_unit_test (NAME api_types_match_real_types
  SOURCES "vmem/gaspi/tests/api_types_match_real_types.cpp"
  LIBRARIES IML::Client
            GASPI::static
            Util::Generic
)

iml_add_unit_test (NAME gaspi-NetdevID
  SOURCES "tests/gaspi/NetdevID.cpp"
  LIBRARIES IML::Client
            Util::Generic
            GASPI::static
)

iml_add_unit_test (NAME dtmmgr
  SOURCES "vmem/tests/dtmmgr.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME tmmgr
  SOURCES "vmem/tests/tmmgr.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME perftmmgr
  SOURCES "vmem/tests/perftmmgr.cpp"
  PERFORMANCE_TEST
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME beegfs_area
  SOURCES "vmem/gaspi/tests/beegfs_area.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
            Boost::filesystem
  NEEDS_BEEGFS
)

iml_add_unit_test (NAME rif-started_process_promise
  SOURCES "rif/test/started_process_promise.cpp"
  DESCRIPTION "pipe used for communicating with a iml-rif-started process during startup"
  LIBRARIES IMLPrivate::rif-started_process_promise
            Util::Generic
)

## System tests

#! Wrapper for \see iml_add_unit_test() and thus shared/cmake's \see
#! add_unit_test() which additionally
#! - Takes \a STARTS_SERVER to add `--iml-vmem-*` arguments and label
#!   the test as `requires_vmem` and implies `RUN_SERIAL`.
#! - Takes \a STARTS_RIFD to add `--iml-rif-*` arguments.
#! - Always adds `NEEDS_INSTALLATION` and `REQUIRED_FILES` for all
#!   files in the installation. The label `requires_installation` is
#!   also always added.
function (iml_add_system_test)
  if (NOT BUILD_TESTING)
    return()
  endif()

  set (options STARTS_SERVER STARTS_RIFD)
  set (one_value_options)
  set (multi_value_options LABELS ARGS)
  set (required_options)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  # When modifying arguments we need to prepend to avoid appending to
  # a different argument (which may not be parsed). `list (PREPEND)`
  # is a CMake 3.15 feature, so use `list (INSERT 0)` instead.

  if (_arg_STARTS_SERVER)
    list (INSERT _arg_ARGS 0
      --iml-vmem-port ${_iml_testing_port_counter}
      --iml-vmem-startup-timeout 60
    )
    math (EXPR _iml_testing_port_counter_tmp
               "${_iml_testing_port_counter} + ${_iml_testing_port_per_test}"
    )
    set (_iml_testing_port_counter ${_iml_testing_port_counter_tmp}
      CACHE INTERNAL
      "Shadow counter for unique --iml-vmem-port arguments."
    )
    list (INSERT _arg_LABELS 0 "requires_vmem")
    list (APPEND _arg_UNPARSED_ARGUMENTS RUN_SERIAL)
  endif()

  if (_arg_STARTS_RIFD)
    list (INSERT _arg_ARGS 0 --iml-rif-strategy ${IML_TESTING_RIF_STRATEGY})

    list (TRANSFORM IML_TESTING_RIF_STRATEGY_PARAMETERS
      PREPEND "--iml-rif-strategy-parameters="
      OUTPUT_VARIABLE _strategy_parameters_args
      )
    list (INSERT _arg_ARGS 0 "${_strategy_parameters_args}")
  endif()

  list (TRANSFORM iml_files_in_installation
    PREPEND "${CMAKE_INSTALL_PREFIX}/"
    OUTPUT_VARIABLE _required_files
  )

  iml_add_unit_test (${_arg_UNPARSED_ARGUMENTS}
    ARGS ${_arg_ARGS}
    LABELS requires_installation ${_arg_LABELS}
    REQUIRED_FILES ${_required_files}
    NEEDS_INSTALLATION
  )
endfunction()

iml_add_system_test (NAME Rifs
  SOURCES "tests/Rifs.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Boost::program_options
  STARTS_RIFD
)

iml_add_system_test (NAME RuntimeSystem
  SOURCES "tests/RuntimeSystem.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Boost::program_options
  STARTS_SERVER
  STARTS_RIFD
)

iml_add_system_test (NAME standalone_put_get
  SOURCES "tests/standalone_put_get.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Util::Generic
            Boost::program_options
  STARTS_SERVER
  STARTS_RIFD
)

## Only run BeeGFS tests if mountpoint qualifies
if (BUILD_TESTING)
  iml_add_executable (NAME iml-check_beegfs_segment_requirements
    SOURCES "vmem/gaspi/tests/check_beegfs_segment_requirements.cpp"
    LIBRARIES IMLPrivate::Server
              Util::Generic
              Boost::program_options
  )

  file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    CONTENT "execute_process (COMMAND \"$<TARGET_FILE:iml-check_beegfs_segment_requirements>\"
                                      \"${IML_TESTING_BEEGFS_DIRECTORY}\"
               WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
               RESULT_VARIABLE iml_check_beegfs_segment_requirements_result
             )

             if (NOT \${iml_check_beegfs_segment_requirements_result} EQUAL 0)
               list (APPEND CTEST_CUSTOM_TESTS_IGNORE ${_iml_testing_beegfs_tests})
             endif()"
  )
endif()

include (${PROJECT_SOURCE_DIR}/meta/iml-meta.cmake OPTIONAL)
