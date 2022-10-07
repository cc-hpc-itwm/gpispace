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

include_guard (GLOBAL)

include (util-cmake/add_macros)
include (util-cmake/add_unit_test)

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
