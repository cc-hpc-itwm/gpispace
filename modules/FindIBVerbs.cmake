# locates the libibverbs library
# This file defines:
# * IBVerbs_FOUND if libibverbs was found
# * IBVerbs_LIBRARY The lib to link to

include (FindPackageHandleStandardArgs)

find_library (IBverbs_LIBRARY
  NAMES ibverbs
  HINTS ${LIBIBVERBS_HOME} ENV LIBIBVERBS_HOME
  PATH_SUFFIXES lib lib64
)

if (NOT IBverbs_LIBRARY)
  message (FATAL_ERROR "IBverbs missing")
endif()

mark_as_advanced (IBverbs_LIBRARY)

find_package_handle_standard_args (IBVerbs REQUIRED_VARS IBverbs_LIBRARY)
