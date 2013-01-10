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
  template.hpp
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
  place_map.fwd.hpp
  port.fwd.hpp
  require.fwd.hpp
  specialize.fwd.hpp
  struct.fwd.hpp
  template.fwd.hpp
  transition.fwd.hpp
  use.fwd.hpp
)

add_xml_sources (
  connect.cpp
  expression.cpp
  function.cpp
  mod.cpp
  net.cpp
  place.cpp
  place_map.cpp
  port.cpp
  require.cpp
  specialize.cpp
  struct.cpp
  template.cpp
  transition.cpp
  use.cpp
)

install (FILES ${xml_parse_type_headers} ${xml_parse_type_fwd_headers}
  DESTINATION include/xml/parse/type
  COMPONENT headers)
