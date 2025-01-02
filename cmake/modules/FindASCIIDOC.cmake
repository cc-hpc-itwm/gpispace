# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

# hint ASCIIDOC_HOME

# executable ASCIIDOC::asciidoc
# function asciidoc

find_path (ASCIIDOC_HOME
  NAMES "bin/asciidoc"
  HINTS ${ASCIIDOC_HOME} ENV ASCIIDOC_HOME
)

find_program (ASCIIDOC_ASCIIDOC_BINARY
  NAMES "asciidoc"
  HINTS ${ASCIIDOC_HOME}
  PATH_SUFFIXES "bin"
)

execute_process (COMMAND ${ASCIIDOC_ASCIIDOC_BINARY} --version
  OUTPUT_VARIABLE ASCIIDOC_version_output
  ERROR_VARIABLE ASCIIDOC_version_error
  RESULT_VARIABLE ASCIIDOC_version_result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (NOT ${ASCIIDOC_version_result} EQUAL 0)
  message (SEND_ERROR "Command \"${ASCIIDOC_ASCIIDOC_BINARY} --version\" failed with output:\n${ASCIIDOC_version_error}")
else()
  if ("${ASCIIDOC_version_output}" MATCHES "^asciidoc [0-9]+[.][0-9]+[.][0-9]+")
    string (REGEX REPLACE "^asciidoc ([0-9]+[.][0-9]+[.][0-9]+)" "\\1"
      ASCIIDOC_VERSION "${ASCIIDOC_version_output}"
    )
  else()
    message (SEND_ERROR "Unable to get asciidoc version: Output does not match expected pattern:\n${ASCIIDOC_version_output}")
  endif()
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (ASCIIDOC
  REQUIRED_VARS ASCIIDOC_HOME
                ASCIIDOC_ASCIIDOC_BINARY
  VERSION_VAR ASCIIDOC_VERSION
)

add_imported_executable (NAME asciidoc
  NAMESPACE ASCIIDOC
  LOCATION "${ASCIIDOC_ASCIIDOC_BINARY}"
)

#"BEGIN INCLUDE"
include (util-cmake/parse_arguments)
#"END INCLUDE"

function (asciidoc)
  set (options INSTALL)
  set (one_value_options NAME SOURCE OUTPUT INSTALL_DESTINATION)
  set (multi_value_options DEPENDS)
  set (required_options NAME OUTPUT)
  _parse_arguments (DOC "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  _default_if_unset (DOC_INSTALL_DESTINATION ".")
  _default_if_unset (DOC_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${DOC_NAME}")

  add_custom_command (OUTPUT "${DOC_OUTPUT}"
    COMMAND ${ASCIIDOC_ASCIIDOC_BINARY} -a data-uri -a toc -o "${DOC_OUTPUT}" "${DOC_SOURCE}"
    DEPENDS "${DOC_SOURCE}" ${DOC_DEPENDS}
    )
  add_custom_target (target_${DOC_NAME} ALL DEPENDS "${DOC_OUTPUT}")

  if (DOC_INSTALL)
    install (FILES "${DOC_OUTPUT}" DESTINATION "${DOC_INSTALL_DESTINATION}")
  endif()
endfunction()
