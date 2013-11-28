# locates the 'asciidoc' exectuable
#
# This file defines:
# * ASCIIDOC_FOUND if asciidoc was found
# * ASCIIDOC_EXECUTABLE the path to the 'asciidoc' binary
# * ASCIIDOC_BINDIR the path containg the 'asciidoc' binary
# * A2X_EXECUTABLE the path to the 'asciidoc' binary
# * A2X_BINDIR the path containg the 'asciidoc' binary

include (FindPackageHandleStandardArgs)

find_program (ASCIIDOC_EXECUTABLE NAMES asciidoc PATH_SUFFIXES bin)
find_program (A2X_EXECUTABLE NAMES a2x PATH_SUFFIXES bin)

get_filename_component (ASCIIDOC_BINDIR ${ASCIIDOC_EXECUTABLE} PATH)
get_filename_component (A2X_BINDIR ${A2X_EXECUTABLE} PATH)

mark_as_advanced (ASCIIDOC_EXECUTABLE ASCIIDOC_BINDIR)
mark_as_advanced (A2X_EXECUTABLE A2X_BINDIR)
mark_as_advanced (ASCIIDOC_VERSION)

execute_process (COMMAND ${ASCIIDOC_EXECUTABLE} --version
  OUTPUT_VARIABLE ASCIIDOC_version_output
  ERROR_VARIABLE ASCIIDOC_version_error
  RESULT_VARIABLE ASCIIDOC_version_result
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT ${ASCIIDOC_version_result} EQUAL 0)
  message (SEND_ERROR "Command \"${ASCIIDOC_EXECUTABLE} --version\" failed with output:\n${ASCIIDOC_version_error}")
else()
  if("${ASCIIDOC_version_output}" MATCHES "^asciidoc [0-9]+\\.[0-9]+\\.[0-9]+")
    string(REGEX REPLACE "^asciidoc ([0-9]+\\.[0-9]+\\.[0-9]+)" "\\1"
      ASCIIDOC_VERSION "${ASCIIDOC_version_output}")
  else()
    message (SEND_ERROR "Unable to get asciidoc version: Output does not match expected pattern:\n${ASCIIDOC_version_output}")
  endif()
endif()

find_package_handle_standard_args (asciidoc
  REQUIRED_VARS ASCIIDOC_EXECUTABLE ASCIIDOC_BINDIR A2X_EXECUTABLE A2X_BINDIR
  VERSION_VAR ASCIIDOC_VERSION
)
