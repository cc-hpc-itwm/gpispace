# locates the 'source_highlite' exectuable
#
# This file defines:
# * SOURCE_HIGHLITE_FOUND if source_highlite was found
# * SOURCE_HIGHLITE_EXECUTABLE the path to the 'source_highlite' binary
# * SOURCE_HIGHLITE_BINDIR the path containg the 'source_highlite' binary

include (FindPackageHandleStandardArgs)

find_program (SOURCE_HIGHLITE_EXECUTABLE NAMES source-highlight PATH_SUFFIXES bin)

get_filename_component (SOURCE_HIGHLITE_BINDIR ${SOURCE_HIGHLITE_EXECUTABLE} PATH)

mark_as_advanced (SOURCE_HIGHLITE_EXECUTABLE SOURCE_HIGHLITE_BINDIR)

execute_process (COMMAND ${SOURCE_HIGHLITE_EXECUTABLE} --version
  OUTPUT_VARIABLE SOURCE_HIGHLITE_version_output
  ERROR_VARIABLE SOURCE_HIGHLITE_version_error
  RESULT_VARIABLE SOURCE_HIGHLITE_version_result
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT ${SOURCE_HIGHLITE_version_result} EQUAL 0)
  message (SEND_ERROR "Command \"${SOURCE_HIGHLITE_EXECUTABLE} --version\" failed with output:\n${SOURCE_HIGHLITE_version_error}")
else()
  if("${SOURCE_HIGHLITE_version_output}" MATCHES "^GNU Source-highlight [0-9]+\\.[0-9]+\\.[0-9]+ \\(library: [0-9]+:[0-9]+:[0-9]+\\).*")
    string(REGEX REPLACE "^GNU Source-highlight ([0-9]+\\.[0-9]+\\.[0-9]+) \\(library: [0-9]+:[0-9]+:[0-9]+\\).*" "\\1"
      SOURCE_HIGHLITE_VERSION "${SOURCE_HIGHLITE_version_output}")
  else()
    message (SEND_ERROR "Unable to get source-highlite version: Output does not match expected pattern:\n${SOURCE_HIGHLITE_version_output}")
  endif()
endif()

find_package_handle_standard_args (source_highlite
  REQUIRED_VARS SOURCE_HIGHLITE_EXECUTABLE SOURCE_HIGHLITE_BINDIR
  VERSION_VAR SOURCE_HIGHLITE_VERSION
)
