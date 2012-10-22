# bernd.loerwald@itwm.fraunhofer.de

set (xml_parse_util_headers
  cdata.hpp
  expect.hpp
  id_type.hpp
  id_type_map.hpp
  mk_fstream.hpp
  name_element.hpp
  optional.hpp
  property.hpp
  required.hpp
  show_node_type.hpp
  skip.hpp
  unique.hpp
  valid_name.hpp
  validprefix.hpp
  validstructfield.hpp
  weparse.hpp
)

set (xml_parse_util_fwd_headers
  id_type.fwd.hpp
  id_type_map.fwd.hpp
)

set (xml_parse_util_sources
  id_type.cpp
  id_type_map.cpp
  mk_fstream.cpp
)

add_library (xml_parse_util
  ${xml_parse_util_sources}
)

target_link_libraries (xml_parse_util
  fhg-util
)

install (FILES ${xml_parse_util_headers} ${xml_parse_util_fwd_headers}
  DESTINATION include/xml/parse/util
  COMPONENT headers)
