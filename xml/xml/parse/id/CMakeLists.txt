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

set (xml_parse_id_sources
  types.cpp
  mapper.cpp
  generic.cpp
)

add_library (xml_parse_id
  ${xml_parse_id_sources}
)

target_link_libraries (xml_parse_id
  fhg-util
)

install (FILES ${xml_parse_id_headers} ${xml_parse_id_fwd_headers}
  DESTINATION include/xml/parse/id
  COMPONENT headers)
