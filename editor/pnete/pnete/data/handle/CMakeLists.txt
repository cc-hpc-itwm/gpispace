# bernd.loerwald@itwm.fraunhofer.de

set (handle_SRCS
  base.cpp
  net.cpp
  place.cpp
  transition.cpp
)

add_library ("${LIB_PREFIX}data_handle" ${handle_SRCS})
target_link_libraries ("${LIB_PREFIX}data_handle" fhg-util)
