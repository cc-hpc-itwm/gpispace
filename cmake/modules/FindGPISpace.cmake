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
#  - REVISION=[0-9a-f]{64}           A specific revision of GPISpace
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
# variable GSPC_REVISION, GSPC_REVISION_FILE
# variable GSPC_GIT_SUBMODULES_FILE
# function bundle_GPISpace (DESTINATION)

set (_gspc_do_submodules_check TRUE)

foreach (_component ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
  if ("${_component}" STREQUAL "ALLOW_DIFFERENT_GIT_SUBMODULES")
    set (_gspc_do_submodules_check FALSE)
  endif()

  string (SUBSTRING "${_component}" 0 9 _prefix)
  if ("${_prefix}" STREQUAL "REVISION=")
    string (SUBSTRING "${_component}" 9 -1 _gspc_required_revision)
  endif()
endforeach()

find_path (GSPC_HOME
  NAMES "revision" "lib/libgspc.so" "we/type/value.hpp"
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
find_file (GSPC_REVISION_FILE "revision" PATHS "${GSPC_HOME}")
find_file (GSPC_GIT_SUBMODULES_FILE "git.submodules" PATHS "${GSPC_HOME}")

if (GSPC_REVISION_FILE)
  file (READ "${GSPC_REVISION_FILE}" GSPC_REVISION)
endif()

mark_as_advanced (GSPC_HOME
  GSPC_we-dev_LIBRARY
  GSPC_drts-context_LIBRARY
  GSPC_gspc_LIBRARY
  GSPC_rif_execution_LIBRARY
  GSPC_pnetc_BINARY
  GSPC_INCLUDE_DIR
  GSPC_REVISION_FILE
  GSPC_GIT_SUBMODULES_FILE
  GSPC_XPNET_XSD
  GSPC_REVISION
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
                GSPC_REVISION_FILE
                GSPC_XPNET_XSD
                GSPC_GIT_SUBMODULES_FILE
  VERSION_VAR GSPC_REVISION
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

  if (_gspc_required_revision AND NOT ALLOW_ANY_GPISPACE_VERSION)
    if (NOT "${_gspc_required_revision}" STREQUAL "${GSPC_REVISION}")
      message (FATAL_ERROR "Found GPISpace with wrong revision: "
        "requested = ${_gspc_required_revision}, got = ${GSPC_REVISION}")
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

function (bundle_GPISpace)
  include (parse_arguments)

  set (options)
  set (one_value_options DESTINATION)
  set (multi_value_options COMPONENTS)
  set (required_options DESTINATION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

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
        )
      elseif ("${_component}" STREQUAL "monitoring")
        list (APPEND _parts "bin/gspc-monitor" "bin/gspcmonc")
      else()
        message (FATAL_ERROR "unknown GPISpace component ${_component}")
      endif()
    endforeach()

    set (_libraries)

    foreach (_part ${_parts})
      get_filename_component (_name "${_part}" NAME)
      get_filename_component (_directory "${_part}" DIRECTORY)

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
    install (FILES "${GSPC_HOME}/revision"
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
