# bernd.loerwald@itwm.fraunhofer.de

set (pnete_data_handle_headers
  base.hpp
  net.hpp
  place.hpp
  transition.hpp
)

set (pnete_data_handle_fwd_headers
  base.fwd.hpp
  net.fwd.hpp
  place.fwd.hpp
  transition.fwd.hpp
)

set (handle_SRCS
  base.cpp
  net.cpp
  place.cpp
  transition.cpp
)

add_library ("${LIB_PREFIX}data_handle" ${handle_SRCS})
target_link_libraries ("${LIB_PREFIX}data_handle" fhg-util)
