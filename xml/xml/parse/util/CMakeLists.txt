# bernd.loerwald@itwm.fraunhofer.de

set (xml_parse_util_headers
  cdata.hpp
  expect.hpp
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
)

set (xml_parse_util_sources
  mk_fstream.cpp
  id_type.cpp
)

add_library(xml_parse_util
  ${xml_parse_util_sources}
)

install (FILES ${xml_parse_util_headers} ${xml_parse_util_fwd_headers}
  DESTINATION include/xml/parse/util
  COMPONENT headers)
