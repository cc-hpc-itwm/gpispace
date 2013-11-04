# locates the 'asciidoc' exectuable
#
# This file defines:
# * ASCIIDOC_FOUND if asciidoc was found
# * ASCIIDOC_EXECUTABLE the path to the 'asciidoc' binary
# * ASCIIDOC_BINDIR the path containg the 'asciidoc' binary
# * ASCIIDOC_VERSION the version of 'asciidoc', if found

find_program (ASCIIDOC_EXECUTABLE
  NAMES asciidoc
  PATH_SUFFIXES bin
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (asciidoc
  DEFAULT_MSG ASCIIDOC_EXECUTABLE
)

mark_as_advanced (ASCIIDOC_EXECUTABLE)

get_filename_component (ASCIIDOC_BINDIR
  ${ASCIIDOC_EXECUTABLE} PATH
)
