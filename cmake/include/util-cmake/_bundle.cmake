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

#"BEGIN INCLUDE"
include (util-cmake/install_directory)
#"END INCLUDE"

#! This script is not intended to be used stand-alone but is used by
#! `add_macros.cmake` to bundle into a location and host independent
#! bundle for deployment.

# Bundling allows an installed project to be location independent and
# freely movable, regardless of dependencies being installed globally
# or not. This is useful for separate build and execution environments
# that differ in their configuration. If the build and execution
# environments are identical, there is no need to enable bundling. The
# default is INSTALL_DO_NOT_BUNDLE = OFF, i.e. bundling, for
# convenience and safety.
#
# When using bundling it may be needed to still have the installed
# libraries point to some additional location in the installed project
# for searching, e.g. an GPI-Space application pointing to the bundled
# GPI-Space. This can be done globally via the INSTALL_RPATH_DIRS list
# variable or per target using the RPATH argument of
# extended_add_library()/extended_add_executable().
#
# When bundling,
# - let the build rpath point to all the dependencies
# - use ldd (in bundle.sh) to find all those resolved dependencies
#
# - copy them into libexec/bundle/lib, filtering known system-specific
#   libraries such as glibc and known already-bundling products like
#   GPI-Space (see bundle.sh)
#
# - remove all rpaths from the bundled libraries
#
# - set the rpath to point to libexec/bundle/lib for anything
#   installed. Also let it point to libraries installed (i.e. lib/ and
#   libexec/product/lib/ sometimes). The latter are manually specified
#   in the add macros.
#
# This way it is ensured that exactly the libraries also used when
# building are used in an installed product. That product is also
# self-contained and can be freely moved around.
#
# When not bundling, the rpaths should point to the libraries used
# when building, still, even if LD_LIBRARY_PATH is unset, and of
# course additionally point to the libraries installed by the project.
#
# To get these behaviors, CMake needs to be told twice:
# - target property INSTALL_RPATH_USE_LINK_PATH: This property tells
#   CMake to copy every path in LD_LIBRARY_PATH (this possibly
#   includes unrelated paths, but oh well) and the paths needed to
#   link everything to the target into the rpath. In the bundling case
#   that should not happen, in the non-bundling case that's exactly
#   what's needed.
#
# - CMAKE_<lang>_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH: This
#   should^tm not be required as it is platform/toolchain specific,
#   but there rarely are proper toolchain configurations, which leads
#   to manually installed compilers and their libstdcxx not being
#   found at runtime.
#
#   In the bundle case, that's irrelevant: It was found during build
#   time, so it will be bundled and there is no need to point to the
#   manually installed compiler. In the non-bundle case, it is
#   important and forces CMake to use the implicit link directories.
#
#   Sadly this is not a property per target but a global variable (see
#   cmComputeLinkInformation.cxx), which is something that's hard to
#   set within functions and macros (CMake only allows to write to the
#   "parent" scope, which may be not enough, and has no way to set
#   global variables explicitly). Additionally, it can't be set
#   immediately to allow projects to include add_macros and set
#   INSTALL_DO_NOT_BUNDLE in *their* CMakelists, so setting it is
#   deferred via _ensure_rpath_globals_before_target_addition().
#   CMake does check the cache though in case a variable doesn't
#   exist, so it can be weaseled in by first ensuring there is no
#   variable set -- or it already has the same value -- and then
#   setting a cache variable to "emulate" a global one. It is
#   impossible to unset the existing variable, because of how CMake
#   variable scopes work.

set (INSTALL_DO_NOT_BUNDLE OFF CACHE BOOL "Do not bundle installed targets")
set (INSTALL_RPATH_DIRS "" CACHE STRING
  "List of paths, either relative into $CMAKE_INSTALL_PREFIX or
   absolute, added to RPATH of all targets defined with
   extended_add_library()/extended_add_executable() (in addition to
   automatic ones from bundling, if enabled). Note that the value of
   this variable is used at configuration, not generation time, so
   order of setting the variable and adding targets matters."
)

# Because these variables have to be set in the global scope, they
# can't be ensured lazily at call time of
# _ensure_rpath_globals_before_target_addition(), even if tracking
# whether the variable has the original value or was modified
# already. Enabling languages in the middle of a project (e.g. a
# sub-directory uses CUDA) is not supported without enabling them all
# up-front before calling add_macros the first time.
if (NOT DEFINED _add_macros_first_enabled_languages)
  get_property (_add_macros_first_enabled_languages GLOBAL
    PROPERTY ENABLED_LANGUAGES
  )
endif()

get_property (_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach (_language ${_languages})
  if (NOT DEFINED _was_defined_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH)
    if (DEFINED CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH)
      set (_was_defined_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH true)
      set (_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH
        ${CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH}
      )
    else()
      set (_was_defined_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH false)
      set (_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH false)
    endif()
  endif()
endforeach()

function (_ensure_rpath_globals_before_target_addition)
  get_property (_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  if (NOT "${_languages}" STREQUAL "${_add_macros_first_enabled_languages}")
    message (FATAL_ERROR "When using 'add_macros.cmake', all languages need to \
be enabled before the first include in order to correctly set RPATH globals.
Please either use only `project (... <languages>)` or check options and \
dynamically `enable_language()` as one of the first things in the project, \
before calling `include (util-cmake/add_macros)`.
Enabled at-include=${_add_macros_first_enabled_languages} vs now=${_languages}."
    )
  endif()

  foreach (_language ${_languages})
    # If it is equal, everything is fine. If it isn't and it was not
    # defined, overwrite it via cache. If it isn't and it was defined,
    # it is impossible to overwrite that way.
    #  == if (${_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH} XOR ${INSTALL_DO_NOT_BUNDLE})
    if ((${_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH} AND NOT ${INSTALL_DO_NOT_BUNDLE})
        OR (NOT ${_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH} AND ${INSTALL_DO_NOT_BUNDLE}))
      if (_was_defined_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH)
        message (FATAL_ERROR "CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH=${_orig_CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH} \
but INSTALL_DO_NOT_BUNDLE=${INSTALL_DO_NOT_BUNDLE} would like to overwrite that, which it \
can't due to scope and definition precendence. This combination is \
thus not supported in this environment."
        )
      else()
        set (CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH
          ${INSTALL_DO_NOT_BUNDLE}
          CACHE BOOL
          "Whether CMake should add CMAKE_${_language}_IMPLICIT_LINK_DIRECTORIES
           to a target's RPATH. Needs to be in sync with INSTALL_DO_NOT_BUNDLE!"
          FORCE
        )
        mark_as_advanced (CMAKE_${_language}_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH)
      endif()
    endif()
  endforeach()
endfunction()

set (util_cmake_bundle_sh "${CMAKE_CURRENT_LIST_DIR}/bundle.sh")

macro (_create_bundle TARGET_NAME BUNDLE_PATH_VAR)
  find_program (CHRPATH_BINARY NAMES chrpath DOC "chrpath is required for bundling")
  if (NOT CHRPATH_BINARY)
    message (FATAL_ERROR "Unable to find chrpath (CHRPATH_BINARY), which is required for "
                         "bundling executables and libraries correctly")
  endif()

  set (_output_path "${CMAKE_CURRENT_BINARY_DIR}/bundle-${TARGET_NAME}")
  set (${BUNDLE_PATH_VAR} "${_output_path}")

  find_package (UnixCommands REQUIRED)

  add_custom_command (OUTPUT "${_output_path}"
    COMMAND "${BASH}" "${util_cmake_bundle_sh}"
    ARGS "${_output_path}"
         "${CHRPATH_BINARY}"
         $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS $<TARGET_FILE:${TARGET_NAME}>
            "${util_cmake_bundle_sh}"
  )
  add_custom_target (${TARGET_NAME}-bundled-libraries ALL
    DEPENDS "${_output_path}"
  )
endmacro()

function (_maybe_bundle_target_and_rpath TARGET_NAME INSTALL_DESTINATION RPATH CREATE_INFO)
  set (_maybe_install_bundle_lib_dir "")

  if (NOT ${INSTALL_DO_NOT_BUNDLE})
    _create_bundle ("${TARGET_NAME}" BUNDLE_PATH)

    install_directory (SOURCE "${BUNDLE_PATH}" DESTINATION "libexec/bundle/lib")

    list (APPEND _maybe_install_bundle_lib_dir "libexec/bundle/lib")
  endif()

  if (${CREATE_INFO})
    set (_bundle_info_dir "${CMAKE_CURRENT_BINARY_DIR}/bundle-info")
    set (_bundle_info_file "${_bundle_info_dir}/${TARGET_NAME}")

    file (MAKE_DIRECTORY "${_bundle_info_dir}")

    if (NOT ${INSTALL_DO_NOT_BUNDLE})
      add_custom_command (OUTPUT "${_bundle_info_file}"
        COMMAND "find" "." "-type" "f"
                "|"
                "sed" "-e" "s,^\.,libexec/bundle/lib,"
                ">" "${_bundle_info_file}"
        WORKING_DIRECTORY "${BUNDLE_PATH}"
        DEPENDS ${TARGET_NAME}-bundled-libraries
      )
      add_custom_target (${TARGET_NAME}-bundled-libraries-info ALL
        DEPENDS "${_bundle_info_file}"
      )
    else()
      #! \note Even if there is no bundling, (empty) bundle
      #! information needs to exist in order not to handle this as
      #! special case at usage of the information.
      file (WRITE "${_bundle_info_file}" "")
    endif()

    install (FILES "${_bundle_info_file}" DESTINATION "libexec/bundle/info")
  endif()

  string (REGEX REPLACE "[^/]+" ".." RPATH_MID "${INSTALL_DESTINATION}")
  foreach (_rpath IN LISTS INSTALL_RPATH_DIRS _maybe_install_bundle_lib_dir)
    if (IS_ABSOLUTE "${_rpath}")
      list (APPEND RPATH "${_rpath}")
    else()
      list (APPEND RPATH "\$ORIGIN/${RPATH_MID}/${_rpath}")
    endif()
  endforeach()

  set_property (TARGET ${TARGET_NAME} APPEND_STRING
    PROPERTY LINK_FLAGS " -Wl,--disable-new-dtags"
  )

  #! \note need proper rpath in build output for bundling
  set_property (TARGET ${TARGET_NAME} PROPERTY SKIP_BUILD_RPATH false)
  set_property (TARGET ${TARGET_NAME} PROPERTY BUILD_WITH_INSTALL_RPATH false)

  set_property (TARGET ${TARGET_NAME} PROPERTY INSTALL_RPATH ${RPATH})
  set_property (TARGET ${TARGET_NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH ${INSTALL_DO_NOT_BUNDLE})
endfunction()
