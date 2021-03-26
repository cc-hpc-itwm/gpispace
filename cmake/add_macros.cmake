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

# Plain and keyword `target_link_libraries()` signatures cannot be mixed.
cmake_policy (SET CMP0023 NEW)

#! \note workaround for https://cmake.org/Bug/view.php?id=9985, which
#! was solved with policy CMP0065 in 3.4.
#! \todo When updating to 3.5 (because 3.4 breaks pkgconfig), remove
#! this, as well as NO_RDYNAMIC handling below and replace with the
#! ENABLE_EXPORTS property for specific binaries only (probably only
#! the worker)
get_property (_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach (language ${_languages})
  string (REPLACE "-rdynamic" ""
    CMAKE_SHARED_LIBRARY_LINK_${language}_FLAGS
    "${CMAKE_SHARED_LIBRARY_LINK_${language}_FLAGS}"
  )
endforeach()

set (_add_macros_bundle_sh "${CMAKE_CURRENT_LIST_DIR}/bundle.sh")
set (_add_macros_test_wrapper "${CMAKE_CURRENT_LIST_DIR}/_test_wrapper.cmake")

macro (_default_if_unset VAR VAL)
  if (NOT ${VAR})
    set (${VAR} ${VAL})
  endif()
endmacro()

#! Bundling allows an installed project to be location independent and
#! freely movable, regardless of dependencies being installed globally
#! or not. This is useful for separate build and execution
#! environments that differ in their configuration. If the build and
#! execution environments are identical, there is no need to enable
#! bundling. The default is INSTALL_DO_NOT_BUNDLE = OFF,
#! i.e. bundling, for convenience and safety.
#!
#! When using bundling it may be needed to still have the installed
#! libraries point to some additional location in the installed
#! project for searching, e.g. an GPI-Space application pointing to
#! the bundled GPI-Space. This can be done globally via the
#! INSTALL_RPATH_DIRS list variable or per target using the RPATH
#! argument of extended_add_library()/extended_add_executable().
#!
#! When bundling,
#! - let the build rpath point to all the dependencies
#! - use ldd (in bundle.sh) to find all those resolved dependencies
#! - copy them into libexec/bundle/lib, filtering known
#!   system-specific libraries such as glibc and known
#!   already-bundling products like GPI-Space (see bundle.sh)
#! - remove all rpaths from the bundled libraries
#! - set the rpath to point to libexec/bundle/lib for anything
#!   installed. Also let it point to libraries installed (i.e. lib/
#!   and libexec/product/lib/ sometimes). The latter are manually
#!   specified in the add macros.
#! This way it is ensured that exactly the libraries also used when
#! building are used in an installed product. That product is also
#! self-contained and can be freely moved around.
#!
#! When not bundling, the rpaths should point to the libraries used
#! when building, still, even if LD_LIBRARY_PATH is unset, and of
#! course additionally point to the libraries installed by the
#! project.
#!
#! To get these behaviors, CMake needs to be told twice:
#! - target property INSTALL_RPATH_USE_LINK_PATH: This property tells
#!   CMake to copy every path in LD_LIBRARY_PATH (this possibly
#!   includes unrelated paths, but oh well) and the paths needed to
#!   link everything to the target into the rpath. In the bundling
#!   case that should not happen, in the non-bundling case that's
#!   exactly what's needed.
#!
#! - CMAKE_<lang>_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH: This
#!   should^tm not be required as it is platform/toolchain specific,
#!   but there rarely are proper toolchain configurations, which leads
#!   to manually installed compilers and their libstdcxx not being
#!   found at runtime.
#!
#!   In the bundle case, that's irrelevant: It was found during build
#!   time, so it will be bundled and there is no need to point to the
#!   manually installed compiler. In the non-bundle case, it is
#!   important and forces CMake to use the implicit link directories.
#!
#!   Sadly this is not a property per target but a global variable
#!   (see cmComputeLinkInformation.cxx), which is something that's
#!   hard to set within functions and macros (CMake only allows to
#!   write to the "parent" scope, which may be not enough, and has no
#!   way to set global variables explicitly). Additionally, it can't
#!   be set immediately to allow projects to include add_macros and
#!   set INSTALL_DO_NOT_BUNDLE in *their* CMakelists, so setting it is
#!   deferred via _ensure_rpath_globals_before_target_addition().
#!   CMake does check the cache though in case a variable doesn't
#!   exist, so it can be weaseled in by first ensuring there is no
#!   variable set -- or it already has the same value -- and then
#!   setting a cache variable to "emulate" a global one. It is
#!   impossible to unset the existing variable, because of how
#!   CMake variable scopes work.

set (INSTALL_DO_NOT_BUNDLE OFF CACHE BOOL "Do not bundle installed targets")
set (INSTALL_RPATH_DIRS "" CACHE STRING
  "List of paths, either relative into $CMAKE_INSTALL_PREFIX or
   absolute, added to RPATH of all targets defined with
   extended_add_library()/extended_add_executable() (in addition to
   automatic ones from bundling, if enabled). Note that the value of
   this variable is used at configuration, not generation time, so
   order of setting the variable and adding targets matters."
)


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
    COMMAND "${BASH}" "${_add_macros_bundle_sh}"
    ARGS "${_output_path}"
         "${CHRPATH_BINARY}"
         $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS $<TARGET_FILE:${TARGET_NAME}>
            "${_add_macros_bundle_sh}"
  )
  add_custom_target (${TARGET_NAME}-bundled-libraries ALL
    DEPENDS "${_output_path}"
  )
endmacro()

function (_maybe_bundle_target_and_rpath TARGET_NAME INSTALL_DESTINATION RPATH CREATE_INFO)
  set (_maybe_install_bundle_lib_dir "")

  if (NOT ${INSTALL_DO_NOT_BUNDLE})
    _create_bundle ("${TARGET_NAME}" BUNDLE_PATH)

    include (install_directory)
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

include (add_cxx_compiler_flag_if_supported)

macro (_moc TARGET_VAR)
  set (HACK_OPTIONS)
  list (APPEND HACK_OPTIONS "-DBOOST_LEXICAL_CAST_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION")

  #! \note directly or indirectly having 'namespace BOOST_JOIN()'
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_AND_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_AND_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_OR_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_OR_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_XOR_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_BIT_XOR_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_COMPLEMENT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_DEREFERENCE_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_DIVIDES_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_DIVIDES_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_EQUAL_TO_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_GREATER_EQUAL_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_GREATER_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LEFT_SHIFT_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LEFT_SHIFT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LESS_EQUAL_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LESS_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LOGICAL_AND_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LOGICAL_NOT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_LOGICAL_OR_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MINUS_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MINUS_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MODULUS_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MODULUS_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MULTIPLIES_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_MULTIPLIES_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_NEGATE_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_NOT_EQUAL_TO_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_PLUS_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_PLUS_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_POST_DECREMENT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_POST_INCREMENT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_PRE_DECREMENT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_PRE_INCREMENT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_RIGHT_SHIFT_ASSIGN_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_RIGHT_SHIFT_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_UNARY_MINUS_HPP_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_TT_HAS_UNARY_PLUS_HPP_INCLUDED")

  if (TARGET Qt4::QtCore)
    qt4_wrap_cpp (${TARGET_VAR} ${ARGN} OPTIONS "${HACK_OPTIONS}")
  endif()
  if (TARGET Qt5::Core)
    qt5_wrap_cpp (${TARGET_VAR} ${ARGN} OPTIONS "${HACK_OPTIONS}")
  endif()

  #! \note Disable warnings for mocced files: They can't be fixed
  #! anyway and since some projects use -Werror, they would make
  #! everything fail compiling.
  add_cxx_compiler_flag_if_supported_source_files (
    FLAG "-Wno-undefined-reinterpret-cast"
    SOURCES ${${TARGET_VAR}}
  )
  add_cxx_compiler_flag_if_supported_source_files (
    FLAG "-Wno-extra-semi-stmt"
    SOURCES ${${TARGET_VAR}}
  )
endmacro()

include (parse_arguments)

function (extended_add_library)
  set (QT_OPTIONS)
  if (TARGET Qt4::QtCore OR TARGET Qt5::Core)
    set (QT_OPTIONS MOC)
  endif()

  set (options POSITION_INDEPENDENT PRECOMPILED INSTALL CREATE_BUNDLE_INFO NO_RDYNAMIC VISIBILITY_HIDDEN)
  set (one_value_options NAME NAMESPACE TYPE INSTALL_DESTINATION)
  set (multi_value_options
    LIBRARIES ${QT_OPTIONS} SOURCES PUBLIC_HEADERS INCLUDE_DIRECTORIES RPATH
    SYSTEM_INCLUDE_DIRECTORIES COMPILE_DEFINITIONS COMPILE_OPTIONS DEPENDS
  )
  set (required_options NAME)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  _default_if_unset (ARG_TYPE "STATIC")
  _default_if_unset (ARG_INSTALL_DESTINATION "lib")

  if (ARG_NAMESPACE)
    set (target_name "${ARG_NAMESPACE}-${ARG_NAME}")
  else()
    set (target_name "${ARG_NAME}")
  endif()

  if (NOT (${ARG_TYPE} STREQUAL "STATIC" OR ${ARG_TYPE} STREQUAL "SHARED" OR ${ARG_TYPE} STREQUAL "MODULE" OR ${ARG_TYPE} STREQUAL "PYTHON_MODULE"))
    message (FATAL_ERROR "Bad library type: ${ARG_TYPE}")
  endif()

  # A large percentage of callers to `extended_add_library()` at some
  # point end up in the context of GPI-Space and dynamically loaded
  # (dlopen-ed + dlclose-ed) modules, in one way or the
  # other. `dlclose` ends up not unloading libraries with `unique`
  # symbols, so in an effort to reduce the amount of times debugging
  # this, enforce that at least this project's libraries are free of
  # them. Also building the libraries that never end up being linked
  # into a workflow module with this flag is collateral damage that
  # isn't as bad as the weeks that have been wasted with this over the
  # past years.
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list (APPEND ARG_COMPILE_OPTIONS INTERFACE -fno-gnu-unique)
  endif()

  set (_scope_specifier)
  if ((NOT ARG_SOURCES AND NOT ARG_MOC) OR ARG_PRECOMPILED)
    set (_scope_specifier INTERFACE)

    _ensure_rpath_globals_before_target_addition()
    add_library (${target_name} INTERFACE)

    if (ARG_PRECOMPILED)
      if (${ARG_TYPE} STREQUAL "STATIC")
        list (APPEND ARG_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib${target_name}.a")
      else()
        list (APPEND ARG_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib${target_name}.so")
      endif()
    endif()

    target_link_libraries (${target_name} INTERFACE ${ARG_LIBRARIES})
  else()
    set (_scope_specifier PUBLIC)

    _moc (${ARG_NAME}_mocced ${ARG_MOC})

    if (${ARG_TYPE} STREQUAL "PYTHON_MODULE")
      _ensure_rpath_globals_before_target_addition()
      python_add_module (${target_name} ${${ARG_NAME}_mocced} ${ARG_SOURCES})
    else()
      _ensure_rpath_globals_before_target_addition()
      add_library (${target_name} ${ARG_TYPE} ${${ARG_NAME}_mocced} ${ARG_SOURCES})
    endif()

    target_link_libraries (${target_name} PUBLIC ${ARG_LIBRARIES})

    if (NOT ARG_NO_RDYNAMIC)
      set_property (TARGET ${target_name} APPEND_STRING
        PROPERTY LINK_FLAGS " -rdynamic"
        )
    endif()
  endif()
  if (ARG_NAMESPACE)
    add_library (${ARG_NAMESPACE}::${ARG_NAME} ALIAS ${target_name})
  endif()
  if (ARG_PUBLIC_HEADERS)
    set_property (TARGET ${target_name} APPEND
      PROPERTY PUBLIC_HEADER ${ARG_PUBLIC_HEADERS}
    )
  endif()

  if (ARG_SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} SYSTEM
      ${ARG_SYSTEM_INCLUDE_DIRECTORIES})
  endif()
  if (ARG_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} ${ARG_INCLUDE_DIRECTORIES})
  endif()

  if (ARG_POSITION_INDEPENDENT)
    set_property (TARGET ${target_name} APPEND
      PROPERTY COMPILE_FLAGS -fPIC
    )
  endif()

  if (ARG_DEPENDS)
    add_dependencies (${target_name} ${ARG_DEPENDS})
  endif()

  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions (${target_name} ${_scope_specifier} ${ARG_COMPILE_DEFINITIONS})
  endif()

  if (ARG_COMPILE_OPTIONS)
    target_compile_options (${target_name} ${ARG_COMPILE_OPTIONS})
  endif()

  if (ARG_VISIBILITY_HIDDEN)
    if (${ARG_TYPE} STREQUAL "STATIC")
      message (FATAL_ERROR
        "VISIBILITY_HIDDEN makes no sense for static targets"
      )
    endif()
    set_target_properties (${target_name}
      PROPERTIES C_VISIBILITY_PRESET hidden
                 CXX_VISIBILITY_PRESET hidden
                 VISIBILITY_INLINES_HIDDEN true
    )
  endif()

  if (ARG_INSTALL)
    install (TARGETS ${target_name}
      LIBRARY DESTINATION "${ARG_INSTALL_DESTINATION}"
      ARCHIVE DESTINATION "${ARG_INSTALL_DESTINATION}"
    )

    if (NOT ${ARG_TYPE} STREQUAL "STATIC")
      _maybe_bundle_target_and_rpath ("${target_name}" "${ARG_INSTALL_DESTINATION}" "${ARG_RPATH}" ${ARG_CREATE_BUNDLE_INFO})
    endif()
  endif()
endfunction()

function (extended_add_executable)
  set (QT_OPTIONS)
  if (TARGET Qt4::QtCore OR TARGET Qt5::Core)
    set (QT_OPTIONS MOC)
  endif()

  set (options INSTALL DONT_APPEND_EXE_SUFFIX CREATE_BUNDLE_INFO NO_RDYNAMIC STATIC)
  set (one_value_options NAME INSTALL_DESTINATION)
  set (multi_value_options LIBRARIES ${QT_OPTIONS} SOURCES RPATH DEPENDS
    COMPILE_DEFINITIONS INCLUDE_DIRECTORIES SYSTEM_INCLUDE_DIRECTORIES)
  set (required_options NAME SOURCES)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  _default_if_unset (ARG_INSTALL_DESTINATION "bin")

  if (NOT ARG_DONT_APPEND_EXE_SUFFIX)
    set (target_name ${ARG_NAME}.exe)
  else()
    set (target_name ${ARG_NAME})
  endif()

  _moc (${ARG_NAME}_mocced ${ARG_MOC})

  _ensure_rpath_globals_before_target_addition()
  add_executable (${target_name} ${${ARG_NAME}_mocced} ${ARG_SOURCES})
  target_link_libraries (${target_name} PRIVATE ${ARG_LIBRARIES})

  if (ARG_SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} SYSTEM
      ${ARG_SYSTEM_INCLUDE_DIRECTORIES}
    )
  endif()
  if (ARG_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} ${ARG_INCLUDE_DIRECTORIES})
  endif()

  if (NOT ARG_NO_RDYNAMIC)
    set_property (TARGET ${target_name} APPEND_STRING
      PROPERTY LINK_FLAGS " -rdynamic"
      )
  endif()
  if (ARG_STATIC)
    set_property (TARGET ${target_name} APPEND_STRING
      PROPERTY LINK_FLAGS " -static"
      )
  endif()

  if (ARG_DEPENDS)
    add_dependencies (${target_name} ${ARG_DEPENDS})
  endif()

  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions (${target_name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
  endif()

  if (ARG_INSTALL)
    install (TARGETS ${target_name} RUNTIME DESTINATION "${ARG_INSTALL_DESTINATION}")

    string (REGEX REPLACE "[^/]+" ".." PATH_MID "${ARG_INSTALL_DESTINATION}")
    target_compile_definitions (${target_name} PRIVATE "-DINSTALLATION_HOME=\"${PATH_MID}\"")

    _maybe_bundle_target_and_rpath ("${target_name}" "${ARG_INSTALL_DESTINATION}" "${ARG_RPATH}" ${ARG_CREATE_BUNDLE_INFO})
  endif()
endfunction()

function (add_imported_executable)
  set (options)
  set (one_value_options NAME NAMESPACE LOCATION)
  set (multi_value_options)
  set (required_options NAME LOCATION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (ARG_NAMESPACE)
    set (target_name ${ARG_NAMESPACE}::${ARG_NAME})
  else()
    set (target_name ${ARG_NAME})
  endif()

  add_executable (${target_name} IMPORTED)
  set_property (TARGET ${target_name} PROPERTY IMPORTED_LOCATION ${ARG_LOCATION})
endfunction()

# - \a PRE_TEST_HOOK and \a POST_TEST_HOOK can be used to specify
#   CMake scripts to be executed before/after the actual test. Their
#   results are ignored so that they are not abused as pre/post-test
#   conditions that should be in the actual test. Their output is not
#   suppressed though. The post hook is run independent of test result.
#   \todo Timeouts are *not* taken into account, so the post hook will
#   not be executed if the test hangs!
function (add_unit_test)
  set (QT_OPTIONS)
  if (TARGET Qt4::QtCore OR TARGET Qt5::Core)
    set (QT_OPTIONS MOC)
  endif()

  set (options USE_BOOST PERFORMANCE_TEST RUN_SERIAL NO_RDYNAMIC)
  set (one_value_options NAME DESCRIPTION PRE_TEST_HOOK POST_TEST_HOOK)
  set (multi_value_options LIBRARIES ${QT_OPTIONS} SOURCES INCLUDE_DIRECTORIES
    SYSTEM_INCLUDE_DIRECTORIES ARGS DEPENDS COMPILE_FLAGS LABELS REQUIRED_FILES
    COMPILE_DEFINITIONS)
  set (required_options NAME SOURCES)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (NOT BUILD_TESTING)
    return()
  endif()

  _default_if_unset (ARG_DESCRIPTION "${ARG_NAME}")

  set (target_name ${ARG_NAME}.test)

  _moc (${ARG_NAME}_mocced ${ARG_MOC})

  _ensure_rpath_globals_before_target_addition()
  add_executable (${target_name} ${${ARG_NAME}_mocced} ${ARG_SOURCES})

  if (NOT ARG_NO_RDYNAMIC)
    set_property (TARGET ${target_name} APPEND_STRING
      PROPERTY LINK_FLAGS " -rdynamic"
      )
  endif()

  if (ARG_USE_BOOST)
    include (beautify_find_boost)
    find_boost (1.59 REQUIRED QUIET COMPONENTS unit_test_framework)

    list (APPEND ARG_LIBRARIES Boost::unit_test_framework)
    target_compile_definitions (${target_name} PRIVATE
      "-DBOOST_TEST_MODULE=\"${ARG_DESCRIPTION}\""
    )

    if (Boost_VERSION VERSION_EQUAL 1.60 OR Boost_VERSION VERSION_GREATER 1.60)
      list (INSERT ARG_ARGS 0 "--")
    endif()
  endif()

  if (ARG_SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} SYSTEM
      ${ARG_SYSTEM_INCLUDE_DIRECTORIES})
  endif()
  if (ARG_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} ${ARG_INCLUDE_DIRECTORIES})
  endif()

  target_link_libraries (${target_name} PRIVATE ${ARG_LIBRARIES})
  if (NOT ARG_PRE_TEST_HOOK AND NOT ARG_POST_TEST_HOOK)
    add_test (NAME ${ARG_NAME} COMMAND
      $<TARGET_FILE:${target_name}> ${ARG_ARGS}
    )
  else()
    add_test (NAME ${ARG_NAME} COMMAND
      "${CMAKE_COMMAND}"
        -D "test_command=$<TARGET_FILE:${target_name}>"
        -D "test_args=${ARG_ARGS}"
        -D "pre_test_hook=${ARG_PRE_TEST_HOOK}"
        -D "post_test_hook=${ARG_POST_TEST_HOOK}"
        -P "${_add_macros_test_wrapper}"
    )
    set_tests_properties (${ARG_NAME} PROPERTIES
      FAIL_REGULAR_EXPRESSION "### Test failed with exit code .*[.] ###"
    )
  endif()

  if (ARG_DEPENDS)
    add_dependencies (${target_name} ${ARG_DEPENDS})
  endif()

  if (ARG_PERFORMANCE_TEST)
    list (APPEND ARG_LABELS "performance_test")
    set (ARG_RUN_SERIAL 1)
  endif()

  if (ARG_COMPILE_FLAGS)
    set_property (TARGET ${target_name} PROPERTY COMPILE_FLAGS ${ARG_COMPILE_FLAGS})
  endif()

  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions (${target_name} ${ARG_COMPILE_DEFINITIONS})
  endif()

  if (ARG_RUN_SERIAL)
    set_property (TEST ${ARG_NAME} APPEND PROPERTY RUN_SERIAL 1)
  endif()

  if (ARG_LABELS)
    set_property (TEST ${ARG_NAME} APPEND PROPERTY LABELS ${ARG_LABELS})
  endif()

  if (ARG_REQUIRED_FILES)
    set_property (TEST ${ARG_NAME} PROPERTY REQUIRED_FILES ${ARG_REQUIRED_FILES})
  endif()
endfunction()
