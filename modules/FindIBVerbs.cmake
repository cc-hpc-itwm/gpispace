# locates the libibverbs library
# This file defines:
# * IBVerbs_FOUND if libibverbs was found
# * IBVerbs_LIBRARY The lib to link to

include (FindPackageHandleStandardArgs)

find_library (IBVerbs_LIBRARY
  NAMES ibverbs
  HINTS ${IBVERBS_HOME} ENV IBVERBS_HOME
  PATH_SUFFIXES lib lib64
  )

mark_as_advanced (IBVerbs_LIBRARY)

find_package_handle_standard_args (IBVerbs REQUIRED_VARS IBVerbs_LIBRARY)
