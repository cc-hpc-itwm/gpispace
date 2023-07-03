# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

find_package (UnixCommands REQUIRED)
find_program (CHRPATH_BINARY NAMES chrpath
  DOC "chrpath is required for bundling")
if (NOT CHRPATH_BINARY)
  message (FATAL_ERROR "Unable to find chrpath (CHRPATH_BINARY), which is
required for bundling executables and libraries correctly")
endif()

set (_add_macros_bundle_sh "${CMAKE_CURRENT_LIST_DIR}/bundle.sh")

# \todo sync with bin/run.cpp
set (_bundle_gspc_dir "libexec/bundle/gpispace")
set (_bundle_lib_dir "libexec/bundle/lib")

function (_swh_deployment_bundle_libraries _arg_TARGET _arg_INSTALL_DESTINATION)
  # A path relative to the installed target up to the installation root.
  string (REGEX REPLACE "[^/]+" ".." _rpath_mid "${_arg_INSTALL_DESTINATION}")

  set_target_properties (${_arg_TARGET} PROPERTIES
    # Ensure the targets have a working rpath to all dependencies
    # while in the build tree, so that bundle.sh can rely on it.
    SKIP_BUILD_RPATH false
    # Let CMake decide the build rpath independent of the install rpath.
    BUILD_WITH_INSTALL_RPATH false
    # Let the install rpath point to the bundled libraries.
    INSTALL_RPATH "\$ORIGIN/${_rpath_mid}/${_bundle_lib_dir}"
  )

  # Collect all libraries via bundle.sh into one folder per target so
  # that they can be done in parallel and failures are
  # independent. Merge it all together for sake of keeping the
  # installation small. The dependencies should not differ in version
  # between targets within the same build tree anyway.
  set (_output_path "${CMAKE_CURRENT_BINARY_DIR}/bundle-${_arg_TARGET}")
  # \todo ninja refuses to clean _output_path because it isn't empty
  # but it also doesn't know about the byproducts. The byproducts
  # aren't known at configure time though, so can't be added here. Use
  # a custom install script instead?
  add_custom_command (OUTPUT "${_output_path}"
    COMMAND "${BASH}" "${_add_macros_bundle_sh}"
    ARGS "${_output_path}"
         "${CHRPATH_BINARY}"
         $<TARGET_FILE:${_arg_TARGET}>
    DEPENDS $<TARGET_FILE:${_arg_TARGET}>
            "${_add_macros_bundle_sh}"
  )
  add_custom_target (${_arg_TARGET}-bundled-libraries ALL
    DEPENDS "${_output_path}"
  )
  install (DIRECTORY "${_output_path}/"
    DESTINATION "${_bundle_lib_dir}"
    USE_SOURCE_PERMISSIONS
  )
endfunction()

macro (_swh_deployment_gpispace)
  set (_gspc_bundled_components "runtime")
  if (GSPC_WITH_MONITOR_APP)
    list (APPEND _gspc_bundled_components "monitoring")
  endif()

  bundle_GPISpace (DESTINATION "${_bundle_gspc_dir}"
    COMPONENTS ${_gspc_bundled_components}
  )
endmacro()

macro (_swh_deployment_implementation_module _arg_TARGET)
  _swh_deployment_bundle_libraries ("${_arg_TARGET}" "implementation")
  bundle_GPISpace_add_rpath (TARGET ${_arg_TARGET}
    INSTALL_DIRECTORY "implementation"
  )
endmacro()

macro (_swh_deployment_implementation_exe _arg_TARGET)
  _swh_deployment_bundle_libraries ("${_arg_TARGET}" "bin")
  bundle_GPISpace_add_rpath (TARGET ${_arg_TARGET}
    INSTALL_DIRECTORY "bin"
  )
endmacro()

# Don't let CMake automatically append any system or dependency paths
# into the binaries. Every part of the path should be under control.
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH false)
set (CMAKE_CXX_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH false)

# Use RPATH instead of RUNPATH to avoid LD_LIBRARY_PATH overrides.
add_link_options (LINKER:--disable-new-dtags)
