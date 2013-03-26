set (xml_parse_rapidxml_headers
  types.hpp
)

install (FILES ${xml_parse_rapidxml_headers}
  DESTINATION include/xml/rapidxml
  COMPONENT headers)

add_subdirectory (1.13)

