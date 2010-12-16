set (hdrs
  cdata.hpp
  expect.hpp
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
  mk_fstream.hpp
  )

install (FILES ${hdrs}
  DESTINATION include/xml/parse/util
  COMPONENT headers)
