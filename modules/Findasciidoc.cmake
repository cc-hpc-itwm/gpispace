# locates the 'asciidoc' exectuable
#
# This file defines:
# * ASCIIDOC_FOUND if asciidoc was found
# * ASCIIDOC_EXECUTABLE the path to the 'asciidoc' binary
# * ASCIIDOC_BINDIR the path containg the 'asciidoc' binary
# * A2X_FOUND if asciidoc was found
# * A2X_EXECUTABLE the path to the 'asciidoc' binary
# * A2X_BINDIR the path containg the 'asciidoc' binary

include (FindPackageHandleStandardArgs)

find_program (ASCIIDOC_EXECUTABLE
  NAMES asciidoc
  PATH_SUFFIXES bin
)
find_package_handle_standard_args (asciidoc
  DEFAULT_MSG ASCIIDOC_EXECUTABLE
)
mark_as_advanced (ASCIIDOC_EXECUTABLE)
get_filename_component (ASCIIDOC_BINDIR
  ${ASCIIDOC_EXECUTABLE} PATH
)

find_program (A2X_EXECUTABLE
  NAMES a2x
  PATH_SUFFIXES bin
)
find_package_handle_standard_args (a2x
  DEFAULT_MSG A2X_EXECUTABLE
)
mark_as_advanced (A2X_EXECUTABLE)
get_filename_component (A2X_BINDIR
  ${A2X_EXECUTABLE} PATH
)
