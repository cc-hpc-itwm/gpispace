# This file defines:
# * IBverbs_FOUND if libibverbs was found
# * IBverbs_LIBRARY The lib to link to (currently only a static unix lib)

find_library (IBverbs_LIBRARY
  NAMES ibverbs
  HINTS ${LIBIBVERBS_HOME} ENV LIBIBVERBS_HOME
  PATH_SUFFIXES lib lib64
)

if (IBverbs_LIBRARY)
  set (IBverbs_FOUND TRUE)
endif()
