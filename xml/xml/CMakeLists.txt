
add_subdirectory (parse)

add_library (xml
  parse/error.cpp
  parse/id/generic.cpp
  parse/id/mapper.cpp
  parse/id/types.cpp
  parse/parser.cpp
  parse/state.cpp
  parse/type/connect.cpp
  parse/type/expression.cpp
  parse/type/function.cpp
  parse/type/mod.cpp
  parse/type/net.cpp
  parse/type/place.cpp
  parse/type/place_map.cpp
  parse/type/port.cpp
  parse/type/require.cpp
  parse/type/specialize.cpp
  parse/type/struct.cpp
  parse/type/template.cpp
  parse/type/transition.cpp
  parse/type/use.cpp
  parse/type/link.cpp
  parse/util/mk_fstream.cpp
  parse/util/valid_name.cpp
  parse/util/cdata.cpp
  parse/util/expect.cpp
  parse/util/name_element.cpp
  parse/util/optional.cpp
  parse/util/required.cpp
  parse/util/skip.cpp
  parse/util/validprefix.cpp
  parse/util/validstructfield.cpp
  parse/util/property.cpp
  parse/util/show_node_type.cpp
  parse/util/weparse.cpp
  parse/warning.cpp
)
target_link_libraries (xml
  we
  fhg-util
  ${Boost_LIBRARIES}
)

get_target_property(GEN_INCLUDE_EXE gen-includes LOCATION)

set (headerlist_hpp ${CMAKE_CURRENT_BINARY_DIR}/parse/headerlist.hpp)

add_custom_command(OUTPUT ${headerlist_hpp}
  COMMAND ${GEN_INCLUDE_EXE} -o ${headerlist_hpp}
  DEPENDS gen-includes ${CMAKE_CURRENT_BINARY_DIR}/parse/includes.hpp
  COMMENT "generating internal pnetc headerlist"
)

set_source_files_properties (
  "${CMAKE_CURRENT_SOURCE_DIR}/parse/parser.cpp"
  PROPERTIES OBJECT_DEPENDS "${headerlist_hpp}"
)

add_subdirectory (tests)
