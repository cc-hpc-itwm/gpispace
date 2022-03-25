#! \note workaround for https://cmake.org/Bug/view.php?id=9985, which

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

macro (_default_if_unset VAR VAL)
  if (NOT ${VAR})
    set (${VAR} ${VAL})
  endif()
endmacro()

#"BEGIN INCLUDE"
include (util-cmake/_bundle)
include (util-cmake/add_cxx_compiler_flag_if_supported)
include (util-cmake/parse_arguments)
#"END INCLUDE"

#! Run the Qt meta-object compiler on the source files in \a ARGN and
#! let the variable \a TARGET_VAR point to the generated source files.
macro (_moc TARGET_VAR)
  # moc and Boost don't go well together due to e.g. namespace names
  # assembled my macros and moc not being a full featured parser that
  # breaks on some template magic. This list is manually assembled to
  # prevent including anything that directly or indirectly has
  # `namespace BOOST_JOIN()` or other things moc chokes on.
  set (HACK_OPTIONS)
  list (APPEND HACK_OPTIONS "-DBOOST_LEXICAL_CAST_INCLUDED")
  list (APPEND HACK_OPTIONS "-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION")
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

  # Disable warnings for mocced files: They can't be fixed anyway and
  # since some projects use -Werror, they would make everything fail
  # compiling.
  add_cxx_compiler_flag_if_supported_source_files (
    FLAG "-Wno-undefined-reinterpret-cast"
    SOURCES ${${TARGET_VAR}}
  )
  add_cxx_compiler_flag_if_supported_source_files (
    FLAG "-Wno-extra-semi-stmt"
    SOURCES ${${TARGET_VAR}}
  )
endmacro()

#! Convenience wrapper around `add_library()` and various properties
#! that can be set.
#!
#! Every library has a \a NAME. A \a NAMESPACE should be given so that
#! there is a modern CMake target `${NAMESPACE}::${NAME}` is
#! created. If no namespace is given, the target will be `${NAME}`
#! only. Prefer always specifying a \a NAMESPACE to benefit from
#! safety checks.
#!
#! There are multiple \a TYPE of libraries: STATIC, SHARED (or MODULE
#! or PYTHON_MODULE), OBJECT or INTERFACE. Not all features are
#! available for all types. The default \a TYPE is STATIC if there are
#! source files given, and INTERFACE otherwise. `-fpic` is not
#! automatically added, so to allow linking STATIC or OBJECT libraries
#! into SHARED targets, \a POSITION_INDEPENDENT needs to be added.
#!
#! The sources to compile into the library are given by \a SOURCES. In
#! case Qt4 or Qt5 is used, \a MOC also exists to list headers to run
#! the meta-object compiler on.
#!
#! Dependencies (includes and libraries) can be added using \a
#! LIBRARIES. Include directories can also be added legacy-style with
#! explicit \a INCLUDE_DIRECTORIES or \a SYSTEM_INCLUDE_DIRECTORIES,
#! but prefer interface targets (if there is none, consider creating
#! one yourself which only sets include directories publicly and then
#! "link" against that). Compilation flags and definitions can be
#! added using \a COMPILE_DEFINITIONS and \a COMPILE_OPTIONS. All
#! these support the CMake `PRIVATE`/`PUBLIC`/`INTERFACE`
#! semantics. Prefer specifying `PRIVATE` as much as possible.
#!
#! \a VISIBILITY_HIDDEN should be set for most libraries. This changes
#! the default of "export all symbols" to "export only those symbols
#! that are explicitly marked as such", leading to cleaner APIs (use
#! util-generic's `FHG_UTIL_DLLEXPORT`, CMake's `GenerateExportHeader`
#! module or alike).
#! By default `-rdynamic`/`--export-dynamic` is added. This can be
#! disabled by adding \a NO_RDYNAMIC. This is usually a good idea but
#! is default-disabled for legacy compatibility.
#!
#! To install the library, add \a INSTALL. By default it will be
#! installed to `lib/`, but \a INSTALL_DESTINATION can overwrite
#! that. Headers for the library can be automatically installed by
#! listing them in \a PUBLIC_HEADERS.
#!
#! To find dependencies once installed, the dynamic libraries required
#! by the target are by default bundled into `libexec/bundle/lib`,
#! unless the global variable \c INSTALL_DO_NOT_BUNDLE is set to
#! true. Also see the in-depth description of bundling in the file
#! `_bundle.cmake`. The bundled libraries will be automatically added
#! to the rpath of the installed library. Additional entries can be
#! added with \a RPATH or via the \c INSTALL_RPATH_DIRS global. To
#! allow for re-bundling of installed libraries, \a CREATE_BUNDLE_INFO
#! can be added which adds the list of detected dynamic dependencies
#! is written to the text file `libexec/bundle/info/${target_name}`
#! (one library per line). Note that the "build rpath", i.e. the rpath
#! used before installing, is **not** modified but left up to CMake.
#!
#! Build-order-only dependencies can be added with \a DEPENDS.
#!
#!
#! \note The automatically generated underlying target is named
#! `${NAME}` if no \a NAMESPACE is given, else `${NAMESPACE}-${NAME}`.
#! \note If a GCC compiler is used, `-fno-gnu-unique` is added and
#! can't be disabled.
#! \note `INTERFACE` libraries shall not have \a SOURCES or \a MOC and
#! can't be \a POSITION_INDEPENDENT or \a NO_RDYNAMIC. They can't \a
#! INSTALL either.
function (extended_add_library)
  set (QT_OPTIONS)
  if (TARGET Qt4::QtCore OR TARGET Qt5::Core)
    set (QT_OPTIONS MOC)
  endif()

  set (options POSITION_INDEPENDENT INSTALL CREATE_BUNDLE_INFO NO_RDYNAMIC VISIBILITY_HIDDEN)
  set (one_value_options NAME NAMESPACE TYPE INSTALL_DESTINATION)
  set (multi_value_options
    LIBRARIES ${QT_OPTIONS} SOURCES PUBLIC_HEADERS INCLUDE_DIRECTORIES RPATH
    SYSTEM_INCLUDE_DIRECTORIES COMPILE_DEFINITIONS COMPILE_OPTIONS DEPENDS
  )
  set (required_options NAME)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (NOT ARG_TYPE)
    if (NOT ARG_SOURCES AND NOT ARG_MOC)
      set (ARG_TYPE "INTERFACE")
    else()
      set (ARG_TYPE "STATIC")
    endif()
  endif()
  _default_if_unset (ARG_INSTALL_DESTINATION "lib")

  if (ARG_NAMESPACE)
    set (target_name "${ARG_NAMESPACE}-${ARG_NAME}")
  else()
    set (target_name "${ARG_NAME}")
  endif()

  if (NOT (${ARG_TYPE} STREQUAL "STATIC" OR ${ARG_TYPE} STREQUAL "SHARED" OR ${ARG_TYPE} STREQUAL "MODULE" OR ${ARG_TYPE} STREQUAL "PYTHON_MODULE" OR ${ARG_TYPE} STREQUAL "OBJECT" OR ${ARG_TYPE} STREQUAL "INTERFACE"))
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
  if ("${ARG_TYPE}" STREQUAL "INTERFACE")
    set (_scope_specifier INTERFACE)

    _ensure_rpath_globals_before_target_addition()
    add_library (${target_name} INTERFACE)

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

    # Static libraries don't have an rpath and can't describe their
    # dependencies, so don't even try.
    if (NOT ${ARG_TYPE} STREQUAL "STATIC")
      _maybe_bundle_target_and_rpath ("${target_name}" "${ARG_INSTALL_DESTINATION}" "${ARG_RPATH}" ${ARG_CREATE_BUNDLE_INFO})
    endif()
  endif()
endfunction()

#! Convenience wrapper around `add_executable()` and various
#! properties that can be set.
#!
#! Every executable has a \a NAME. By default `.exe` is appended to
#! the name, which can be disabled with \a DONT_APPEND_EXE_SUFFIX.
#!
#! The sources to compile into the executable are given by \a
#! SOURCES. In case Qt4 or Qt5 is used, \a MOC also exists to list
#! headers to run the meta-object compiler on.
#!
#! Dependencies (includes and libraries) can be added using \a
#! LIBRARIES. Include directories can also be added legacy-style with
#! explicit \a INCLUDE_DIRECTORIES or \a SYSTEM_INCLUDE_DIRECTORIES,
#! but prefer interface targets (if there is none, consider creating
#! one yourself which only sets include directories publicly and then
#! "link" against that). Compilation flags and definitions can be
#! added using \a COMPILE_DEFINITIONS. All these support the CMake
#! `PRIVATE`/`PUBLIC`/`INTERFACE` semantics. Prefer specifying
#! `PRIVATE` as much as possible.
#!
#! By default `-rdynamic`/`--export-dynamic` is added. This can be
#! disabled by adding \a NO_RDYNAMIC. This is usually a good idea but
#! is default-disabled for legacy compatibility.
#!
#! An entirely static executable can be requested by adding \a
#! STATIC. Make sure that all libraries linked into it are also
#! static.
#!
#! To install the executable, add \a INSTALL. By default it will be
#! installed to `bin/`, but \a INSTALL_DESTINATION can overwrite that.
#! To allow the binary to find the installation root,
#! `-DINSTALLATION_HOME` is automatically defined to be a relative
#! path from the (installed) binary to the installation root (".." if
#! "bin/", "../.." if "libexec/proj/" etc).
#!
#! To find dependencies, the dynamic libraries required by the target
#! are by default bundled into `libexec/bundle/lib`, unless the global
#! variable \c INSTALL_DO_NOT_BUNDLE is set to true. Also see the
#! in-depth description of bundling in the file `_bundle.cmake`. The
#! bundled libraries will be automatically added to the rpath of the
#! installed executable. Additional entries can be added with \a
#! RPATH. To allow for re-bundling of installed libraries, \a
#! CREATE_BUNDLE_INFO can be added which adds the list of detected
#! dynamic dependencies is written to the text file
#! `libexec/bundle/info/${target_name}` (one executable per line).
#!
#! Build-order-only dependencies can be added with \a DEPENDS.
#!
#! \note The automatically generated underlying target is named
#! `${NAME}.exe` or just `${NAME}` if \a DONT_APPEND_EXE_SUFFIX.
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

#! Add a target for the binary at \a LOCATION with the name \a NAME
#! (`${NAME}`), optionally in \a NAMESPACE (`${NAMESPACE}::${NAME}`).
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
