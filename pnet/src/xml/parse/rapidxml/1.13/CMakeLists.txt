
set (hdrs
  rapidxml.hpp
  rapidxml_iterators.hpp
  rapidxml_print.hpp
  rapidxml_utils.hpp
  )

install (FILES ${hdrs}
  DESTINATION include/xml/parse/rapidxml/1.13/
  COMPONENT headers)

