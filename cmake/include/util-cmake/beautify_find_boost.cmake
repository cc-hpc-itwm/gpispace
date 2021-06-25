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
include (util-cmake/add_macros)
include (util-cmake/parse_arguments)
include (util-cmake/install_directory)
#"END INCLUDE"

#! Wrap `find_package (Boost COMPONENTS x)` with extension to
#! - force use of a GPISpace bundled Boost with \a
#!   FROM_GPISPACE_INSTALLATION (requires a successful `find_package (GPISpace)`)
#! - request \a STATIC libraries if available
#! - create interface targets even for old versions of Boost.
#! - automatically find inter-component dependencies as well in old
#!   versions of Boost (e.g. filesystem requires system, so
#!   `COMPONENTS filesystem` will be enough to successfully link
#!   `Boost::filesystem` as `system` is added automatically).
#! - prevent unavoidable warnings when using Boost.ASIO between
#!  versions 1.61.0 and 1.70.0.
#! - correctly use `Boost::unit_test_framework` in case it is a shared
#!   library.
#! - prevent leaking `UNIQUE` Boost.ProgramOptions symbols.
function (find_boost)
  set (options FROM_GPISPACE_INSTALLATION STATIC)
  set (one_value_options)
  set (multi_value_options COMPONENTS)
  set (required_options COMPONENTS)
  _parse_arguments_with_unknown (FIND "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_all_already_known TRUE)
  foreach (_component ${FIND_COMPONENTS} base)
    if (NOT TARGET Boost::${_component})
      set (_all_already_known FALSE)
    endif()
  endforeach()
  if (_all_already_known)
    return()
  endif()

  if (FIND_FROM_GPISPACE_INSTALLATION)
    if (NOT GPISpace_FOUND)
      message (FATAL_ERROR "GPISpace was not found (GPISpace_FOUND='${GPISpace_FOUND}').
Use `find_package (GPISpace REQUIRED)` before using `find_boost() with `FROM_GPISPACE_INSTALLATION`."
      )
    endif()
    if (NOT GSPC_HOME)
      message (FATAL_ERROR
        "GSPC_HOME not set but FROM_GPISPACE_INSTALLATION is turned on"
      )
    endif()

    set (BOOST_ROOT "${GSPC_HOME}/external/boost" CACHE PATH "BOOST ROOT")
    set (extra_opts "-Wl,-rpath-link -Wl,${GSPC_HOME}/libexec/bundle/lib")
    set (extra_opts "${extra_opts} -Wl,-rpath -Wl,${GSPC_HOME}/libexec/bundle/lib")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${extra_opts}")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${extra_opts}")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${extra_opts}")
  endif()

  if (FIND_STATIC)
    set (Boost_USE_STATIC_LIBS ON)
    set (Boost_USE_STATIC_RUNTIME ON)
  endif()

  find_package (Boost ${FIND_UNPARSED_ARGUMENTS} COMPONENTS ${FIND_COMPONENTS})

  set (Boost_VERSION ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION})
  set (Boost_VERSION ${Boost_VERSION} PARENT_SCOPE)

  if (NOT TARGET Boost::base)
    extended_add_library (NAME base
      NAMESPACE Boost
      SYSTEM_INCLUDE_DIRECTORIES INTERFACE ${Boost_INCLUDE_DIR}
    )

    #! \note With Boost 1.62, Boost.Coroutine is deprecated and should be
    #! replaced with Boost.Coroutine2. Boost.ASIO does not yet do so. As
    #! soon as it does, this should be extended to limit with an upper
    #! version.
    if (Boost_VERSION VERSION_GREATER 1.61.0 AND Boost_VERSION VERSION_LESS 1.70.0)
      target_compile_definitions (Boost-base INTERFACE
        BOOST_COROUTINES_NO_DEPRECATION_WARNING
      )
    endif ()

    if (Boost_VERSION VERSION_EQUAL 1.62.0)
      target_compile_definitions(Boost-base INTERFACE
        BOOST_COROUTINE_NO_DEPRECATION_WARNING
      )
    endif ()
  endif()

  foreach (component ${FIND_COMPONENTS})
    string (TOUPPER ${component} UPPERCOMPONENT)
    if (Boost_${UPPERCOMPONENT}_FOUND)
      set (additional_libraries Boost::base)
      if (${component} STREQUAL filesystem)
        find_boost (${FIND_UNPARSED_ARGUMENTS} COMPONENTS system)
        list (APPEND additional_libraries Boost::system)
      endif()
      if (${component} STREQUAL thread)
        find_package (Threads REQUIRED)

        list (APPEND additional_libraries Threads::Threads)
        #! \todo find librt, only link it when required
        list (APPEND additional_libraries rt)
      endif()
      if (${component} STREQUAL coroutine)
        find_boost (${FIND_UNPARSED_ARGUMENTS} COMPONENTS context chrono)
        list (APPEND additional_libraries Boost::context Boost::chrono)
      endif()
      if (${component} STREQUAL context)
        find_boost (${FIND_UNPARSED_ARGUMENTS} COMPONENTS chrono)
        list (APPEND additional_libraries Boost::chrono)
      endif()
      if (  Boost_VERSION VERSION_EQUAL 1.59.0
         OR Boost_VERSION VERSION_GREATER 1.59.0
         )
        if (${component} STREQUAL timer)
          find_boost (${FIND_UNPARSED_ARGUMENTS} COMPONENTS chrono)
          list (APPEND additional_libraries Boost::chrono)
        endif()
        if (${component} STREQUAL unit_test_framework)
          find_boost (${FIND_UNPARSED_ARGUMENTS} COMPONENTS timer)
          list (APPEND additional_libraries Boost::timer)
        endif()
      endif()

      if (NOT TARGET Boost::${component})
        extended_add_library (NAME ${component}
          NAMESPACE Boost
          LIBRARIES ${Boost_${UPPERCOMPONENT}_LIBRARIES} ${additional_libraries}
        )
      else()
        set_property (TARGET Boost::${component}
          APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${additional_libraries}
        )
      endif()
    endif()
  endforeach()

  # Boost.ProgramOptions has a static string that has in the past
  # often triggered duplicate-frees when linked directly and
  # indirectly into the same target, or via both .a and .so (e.g. via
  # GPISpace's libgspc.so).
  # In order to prevent this, tell the linker to not re-export any
  # Boost.ProgramOptions symbols, which to our knowledge is fine as
  # there are no symbols that actually require to be singletons.
  if (TARGET Boost::program_options)
    get_property (_current_options
      TARGET Boost::program_options
      PROPERTY INTERFACE_LINK_OPTIONS
    )
    set (_boost_po_link_workaround
      "LINKER:--exclude-libs,libboost_program_options.a"
    )
    if (NOT _boost_po_link_workaround IN_LIST _current_options)
      set_property (TARGET Boost::program_options APPEND
        PROPERTY INTERFACE_LINK_OPTIONS ${_boost_po_link_workaround}
      )
    endif()
  endif()

  # boost::unit_test_framework, when linked using a shared library,
  # needs to be explicitly told to generate `main()` as well. CMake's
  # FindBoost does not do that, so detect what type of library it gave
  # us and fix up that flag if needed.
  # In the past we have incorrectly linked against
  # Boost::test_exec_monitor, which seemed to be fine, but isn't: That
  # results in linking issues with some global variables that end up
  # in segfaults during destruction in some environments.
  if ( TARGET Boost::unit_test_framework
      AND Boost_UNIT_TEST_FRAMEWORK_LIBRARY MATCHES "[.]so[0-9.]*$"
     )
    get_property (_current_options
      TARGET Boost::unit_test_framework
      PROPERTY INTERFACE_COMPILE_DEFINITIONS
    )
    set (_boost_test_shared_link_option BOOST_TEST_DYN_LINK)
    if (NOT _boost_test_shared_link_option IN_LIST _current_options)
      set_property (TARGET Boost::unit_test_framework APPEND
        PROPERTY INTERFACE_COMPILE_DEFINITIONS ${_boost_test_shared_link_option}
      )
    endif()
  endif()
endfunction()

#! Bundle an entire Boost installation into the calling project's
#! installation \a DESTINATION subdirectory.
#! \warn Does **not** know what "Boost" actually is, so bundling a
#! system-installed Boost will bundle **the entire system**.
#! \note Requires Boost to be found beforehand.
function (bundle_boost)
  set (options)
  set (one_value_options DESTINATION)
  set (multi_value_options)
  set (required_options DESTINATION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  get_filename_component (BOOST_ROOT "${Boost_INCLUDE_DIR}" PATH)

  install_directory (SOURCE "${BOOST_ROOT}" DESTINATION "${ARG_DESTINATION}")
endfunction()
