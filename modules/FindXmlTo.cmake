# locates the 'xmlto' exectuable
#
# This file defines:
# * XMLTO_FOUND if xmlto was found
# * XMLTO_EXECUTABLE the path to the 'xmlto' binary

find_program (XMLTO_EXECUTABLE
  NAMES xmlto
  HINTS ${XMLTO_HOME} ENV XMLTO_HOME
  PATH_SUFFIXES bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (XmlTo DEFAULT_MSG XMLTO_EXECUTABLE)

mark_as_advanced (XMLTO_EXECUTABLE)
