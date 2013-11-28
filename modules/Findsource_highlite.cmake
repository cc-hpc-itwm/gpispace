# locates the 'source_highlite' exectuable
#
# This file defines:
# * SOURCE_HIGHLITE_FOUND if source_highlite was found
# * SOURCE_HIGHLITE_EXECUTABLE the path to the 'source_highlite' binary
# * SOURCE_HIGHLITE_BINDIR the path containg the 'source_highlite' binary

find_program (SOURCE_HIGHLITE_EXECUTABLE
  NAMES source-highlight
  PATH_SUFFIXES bin
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (source_highlite
  DEFAULT_MSG SOURCE_HIGHLITE_EXECUTABLE
)

mark_as_advanced (SOURCE_HIGHLITE_EXECUTABLE)

get_filename_component (SOURCE_HIGHLITE_BINDIR
  ${SOURCE_HIGHLITE_EXECUTABLE} PATH
)
