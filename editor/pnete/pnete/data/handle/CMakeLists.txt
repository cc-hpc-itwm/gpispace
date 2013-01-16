# bernd.loerwald@itwm.fraunhofer.de

set (pnete_data_handle_headers
  base.hpp
  connect.hpp
  expression.hpp
  function.hpp
  meta_base.hpp
  module.hpp
  net.hpp
  place.hpp
  port.hpp
  transition.hpp
)

set (pnete_data_handle_fwd_headers
  base.fwd.hpp
  connect.fwd.hpp
  expression.fwd.hpp
  function.fwd.hpp
  meta_base.fwd.hpp
  module.fwd.hpp
  net.fwd.hpp
  place.fwd.hpp
  port.fwd.hpp
  transition.fwd.hpp
)

set (handle_SRCS
  base.cpp
  connect.cpp
  expression.cpp
  function.cpp
  module.cpp
  net.cpp
  place.cpp
  port.cpp
  transition.cpp
)

add_library ("${LIB_PREFIX}data_handle" ${handle_SRCS})
target_link_libraries ("${LIB_PREFIX}data_handle" fhg-util)
