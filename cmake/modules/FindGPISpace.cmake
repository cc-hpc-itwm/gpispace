# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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

# hints GSPC_HOME

# Arguments (via COMPONENTS):
#  - VERSION=[0-9].[0-9]{.[0.9]}      A specific version of GPISpace
#      to use. Note that this can be skipped temporarily by defining
#      the variable `ALLOW_ANY_GPISPACE_VERSION` on the cmake command
#      line.
#  - ALLOW_DIFFERENT_GIT_SUBMODULES  Skip check that git submodules
#      that are used in GPISpace as well are used in the same revision.

# library GPISPace::header-only
# library GPISPace::workflow_development
# library GPISPace::execution
# executable GPISPace::pnetc
# variable GSPC_HOME
# variable GSPC_XPNET_XSD
# variable GSPC_VERSION, GSPC_VERSION_FILE
# variable GSPC_GIT_SUBMODULES_FILE
# function bundle_GPISpace (DESTINATION)

set (_gspc_do_submodules_check TRUE)

foreach (_component ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
  if ("${_component}" STREQUAL "ALLOW_DIFFERENT_GIT_SUBMODULES")
    set (_gspc_do_submodules_check FALSE)
  endif()

  string (SUBSTRING "${_component}" 0 9 _prefix)
  if ("${_prefix}" STREQUAL "VERSION=")
    string (SUBSTRING "${_component}" 9 -1 _gspc_required_version)
  endif()
endforeach()

find_path (GSPC_HOME
  NAMES "version" "lib/libgspc.so" "we/type/value.hpp"
  HINTS ${GSPC_HOME} ENV GSPC_HOME
)

find_library (GSPC_we-dev_LIBRARY
  NAMES "libwe-dev.so"
  HINTS ${GSPC_HOME}
  PATH_SUFFIXES "lib"
)

find_library (GSPC_drts-context_LIBRARY
  NAMES "libdrts-context.so"
  HINTS ${GSPC_HOME}
  PATH_SUFFIXES "lib"
)

find_library (GSPC_gspc_LIBRARY
  NAMES "libgspc.so"
  HINTS ${GSPC_HOME}
  PATH_SUFFIXES "lib"
)

find_library (GSPC_rif_execution_LIBRARY
  NAMES "librif-started_process_promise.a"
  HINTS ${GSPC_HOME}
  PATH_SUFFIXES "lib"
)

find_program (GSPC_pnetc_BINARY
  NAMES "pnetc"
  HINTS ${GSPC_HOME}
  PATH_SUFFIXES "bin"
)

find_file (GSPC_INCLUDE_DIR "include" PATHS "${GSPC_HOME}"
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
)
find_file (GSPC_XPNET_XSD "share/gspc/xml/xsd/pnet.xsd" PATHS "${GSPC_HOME}")
find_file (GSPC_VERSION_FILE "version" PATHS "${GSPC_HOME}")
find_file (GSPC_GIT_SUBMODULES_FILE "git.submodules" PATHS "${GSPC_HOME}")

if (GSPC_VERSION_FILE)
  file (READ "${GSPC_VERSION_FILE}" GSPC_VERSION)
endif()

mark_as_advanced (GSPC_HOME
  GSPC_we-dev_LIBRARY
  GSPC_drts-context_LIBRARY
  GSPC_gspc_LIBRARY
  GSPC_rif_execution_LIBRARY
  GSPC_pnetc_BINARY
  GSPC_INCLUDE_DIR
  GSPC_VERSION_FILE
  GSPC_GIT_SUBMODULES_FILE
  GSPC_XPNET_XSD
  GSPC_VERSION
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GPISpace
  REQUIRED_VARS GSPC_HOME
                GSPC_we-dev_LIBRARY
                GSPC_drts-context_LIBRARY
                GSPC_gspc_LIBRARY
                GSPC_rif_execution_LIBRARY
                GSPC_pnetc_BINARY
                GSPC_INCLUDE_DIR
                GSPC_VERSION_FILE
                GSPC_XPNET_XSD
                GSPC_GIT_SUBMODULES_FILE
  VERSION_VAR GSPC_VERSION
)

if (GPISpace_FOUND)
  include (beautify_find_boost)
  find_boost (1.59 REQUIRED FROM_GPISPACE_INSTALLATION
    COMPONENTS coroutine
               date_time
               filesystem
               iostreams
               program_options
               serialization
               system
               thread
  )

  if (_gspc_required_version AND NOT ALLOW_ANY_GPISPACE_VERSION)
    if (NOT "${_gspc_required_version}" STREQUAL "${GSPC_VERSION}")
      message (FATAL_ERROR "Found GPISpace with wrong version: "
        "requested = ${_gspc_required_version}, got = ${GSPC_VERSION}")
    endif()
  endif()

  if (_gspc_do_submodules_check)
    include (git_submodules)
    list_and_store_git_submodules ("${CMAKE_CURRENT_BINARY_DIR}/git.submodules")
    assert_same_git_submodules ("${CMAKE_CURRENT_BINARY_DIR}/git.submodules" "${GSPC_GIT_SUBMODULES_FILE}")
  endif()

  extended_add_library (NAME header-only
    NAMESPACE GPISpace
    SYSTEM_INCLUDE_DIRECTORIES INTERFACE "${GSPC_INCLUDE_DIR}"
  )

  extended_add_library (NAME workflow_development
    NAMESPACE GPISpace
    LIBRARIES GPISpace::header-only
              ${GSPC_we-dev_LIBRARY}
              ${GSPC_drts-context_LIBRARY}
              Boost::filesystem
              Boost::iostreams
              Boost::serialization
              Boost::system
              Boost::thread
  )

  extended_add_library (NAME execution
    NAMESPACE GPISpace
    LIBRARIES GPISpace::header-only
              ${GSPC_gspc_LIBRARY}
              ${GSPC_rif_execution_LIBRARY}
              ${GSPC_drts-context_LIBRARY}
              Boost::coroutine
              Boost::date_time
              Boost::filesystem
              Boost::program_options
              Boost::serialization
              Boost::thread
  )

  add_imported_executable (NAME pnetc
    NAMESPACE GPISpace
    LOCATION "${GSPC_pnetc_BINARY}"
  )
endif()

set (_gpispace_bundle_destination "")

function (bundle_GPISpace)
  include (parse_arguments)

  set (options)
  set (one_value_options DESTINATION)
  set (multi_value_options COMPONENTS)
  set (required_options DESTINATION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if ("${_gpispace_bundle_destination}" STREQUAL "")
    set (_gpispace_bundle_destination "${ARG_DESTINATION}" PARENT_SCOPE)
  else()
    message (FATAL_ERROR "It is not supported to call bundle_GPISpace() twice.")
  endif()

  if (ARG_COMPONENTS)
    set (_parts)

    foreach (_component ${ARG_COMPONENTS})
      if ("${_component}" STREQUAL "runtime")
        list (APPEND _parts "libexec/gspc/agent"
                            "libexec/gspc/drts-kernel"
                            "libexec/gspc/gpi-space"
                            "bin/gspc-bootstrap-rifd"
                            "bin/gspc-rifd"
                            "bin/gspc-teardown-rifd"
                            "libexec/gspc/gspc-logging-demultiplexer.exe"
                            "lib/libdrts-context.so"
                            "lib/libgspc.so"
                            "lib/libwe-dev.so"
        )
      elseif ("${_component}" STREQUAL "monitoring")
        list (APPEND _parts "bin/gspc-monitor")
      else()
        message (FATAL_ERROR "unknown GPISpace component ${_component}")
      endif()
    endforeach()

    set (_libraries "")

    foreach (_part ${_parts})
      get_filename_component (_name "${_part}" NAME)
      get_filename_component (_directory "${_part}" DIRECTORY)

      string (REGEX REPLACE "^lib(.*)\\.so$" "\\1" _name "${_name}")

      file (READ "${GSPC_HOME}/libexec/bundle/info/${_name}" _info)
      string (REGEX REPLACE ";" "\\\\;" _info "${_info}")
      string (REGEX REPLACE "\n" ";" _info "${_info}")
      foreach (_bundled ${_info})
        list (APPEND _libraries "${GSPC_HOME}/${_bundled}")
      endforeach()

      install (PROGRAMS "${GSPC_HOME}/${_part}"
        DESTINATION "${ARG_DESTINATION}/${_directory}"
      )
    endforeach()

    list (REMOVE_DUPLICATES _libraries)

    install (PROGRAMS ${_libraries}
      DESTINATION "${ARG_DESTINATION}/libexec/bundle/lib"
    )
    install (FILES "${GSPC_HOME}/version"
      DESTINATION "${ARG_DESTINATION}"
    )
  else()
    include (install_directory)
    install_directory (SOURCE "${GSPC_HOME}" DESTINATION "${ARG_DESTINATION}")
  endif()

  if (INSTALL_DO_NOT_BUNDLE)
    list (APPEND INSTALL_RPATH_DIRS "${ARG_DESTINATION}/lib")
    set (INSTALL_RPATH_DIRS "${INSTALL_RPATH_DIRS}" PARENT_SCOPE)
  endif()
endfunction()

# Set RPATH of \a TARGET which is installed to \a INSTALL_DIRECTORY to
# include the bundled GPI-Space.
# \note Any target using GPI-Space with a bundle_GPISpace()-bundled
# GPI-Space installation (rather than pointing to an external one)
# needs to set up RPATH to point into that bundle.
# \note The \a INSTALL_DIRECTORY shall be the *relative* path of the
# target in the installation, i.e. if installing the target Blorb to
# "lib/libBlorb.so", TARGET shall be "Blorb" and INSTALL_DIRECTORY
# shall be "lib".
function (bundle_GPISpace_add_rpath)
  include (parse_arguments)

  set (options)
  set (one_value_options TARGET INSTALL_DIRECTORY)
  set (multi_value_options)
  set (required_options TARGET INSTALL_DIRECTORY)
  _parse_arguments (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if ("${_gpispace_bundle_destination}" STREQUAL "")
    message (FATAL_ERROR "bundle_GPISpace() needs to be called before using "
      "bundle_GPISpace_add_rpath()."
    )
  endif()

  string (REGEX REPLACE "[^/]+" ".." _rpath_mid "${_arg_INSTALL_DIRECTORY}")
  set (_rpath_prefix "\$ORIGIN/${_rpath_mid}/${_gpispace_bundle_destination}")

  get_property (_rpath TARGET ${_arg_TARGET} PROPERTY INSTALL_RPATH)

  list (APPEND _rpath "${_rpath_prefix}/lib")
  list (APPEND _rpath "${_rpath_prefix}/libexec/bundle/lib")

  set_property (TARGET ${_arg_TARGET} PROPERTY INSTALL_RPATH ${_rpath})
endfunction()
