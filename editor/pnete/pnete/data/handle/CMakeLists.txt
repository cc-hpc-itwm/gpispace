# bernd.loerwald@itwm.fraunhofer.de

set (pnete_data_handle_headers
  base.hpp
  function.hpp
  net.hpp
  place.hpp
  port.hpp
  transition.hpp
)

set (pnete_data_handle_fwd_headers
  base.fwd.hpp
  function.fwd.hpp
  net.fwd.hpp
  place.fwd.hpp
  port.fwd.hpp
  transition.fwd.hpp
)

set (handle_SRCS
  base.cpp
  function.cpp
  net.cpp
  place.cpp
  port.cpp
  transition.cpp
)

add_library ("${LIB_PREFIX}data_handle" ${handle_SRCS})
target_link_libraries ("${LIB_PREFIX}data_handle" fhg-util)
