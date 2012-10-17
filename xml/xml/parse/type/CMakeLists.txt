# bernd.loerwald@itwm.fraunhofer.de

set (xml_parse_type_headers
  connect.hpp
  expression.hpp
  function.hpp
  mod.hpp
  net.hpp
  place.hpp
  place_map.hpp
  port.hpp
  require.hpp
  specialize.hpp
  struct.hpp
  token.hpp
  transition.hpp
  use.hpp
)

set (xml_parse_type_fwd_headers
  connect.fwd.hpp
  expression.fwd.hpp
  function.fwd.hpp
  mod.fwd.hpp
  net.fwd.hpp
  place.fwd.hpp
  port.fwd.hpp
  transition.fwd.hpp
)

set (xml_parse_type_sources
  connect.cpp
  expression.cpp
  mod.cpp
  place.cpp
  port.cpp
)

add_library (xml_parse_type
  ${xml_parse_type_sources}
)

install (FILES ${xml_parse_type_headers} ${xml_parse_type_fwd_headers}
  DESTINATION include/xml/parse/type
  COMPONENT headers)
