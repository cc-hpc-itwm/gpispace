# bernd.loerwald@itwm.fraunhofer.de

set (xml_parse_id_headers
  types.hpp
  mapper.hpp
  generic.hpp
)

set (xml_parse_id_fwd_headers
  types.fwd.hpp
  mapper.fwd.hpp
)

add_xml_sources (
  types.cpp
  mapper.cpp
  generic.cpp
)

install (FILES ${xml_parse_id_headers} ${xml_parse_id_fwd_headers}
  DESTINATION include/xml/parse/id
  COMPONENT headers)
