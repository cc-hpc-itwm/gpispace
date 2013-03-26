# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

include (FindThreads)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT})
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

include (FindUTIL REQUIRED)
include_directories (${UTIL_INCLUDE_DIR})

add_subdirectory (apps)
add_subdirectory (we)

add_subdirectory (xml/parse)

add_library (xml
  xml/parse/error.cpp
  xml/parse/id/generic.cpp
  xml/parse/id/mapper.cpp
  xml/parse/id/types.cpp
  xml/parse/parser.cpp
  xml/parse/state.cpp
  xml/parse/type/connect.cpp
  xml/parse/type/expression.cpp
  xml/parse/type/function.cpp
  xml/parse/type/mod.cpp
  xml/parse/type/net.cpp
  xml/parse/type/place.cpp
  xml/parse/type/place_map.cpp
  xml/parse/type/port.cpp
  xml/parse/type/require.cpp
  xml/parse/type/specialize.cpp
  xml/parse/type/struct.cpp
  xml/parse/type/template.cpp
  xml/parse/type/transition.cpp
  xml/parse/type/use.cpp
  xml/parse/type/link.cpp
  xml/parse/type/with_position_of_definition.cpp
  xml/parse/util/mk_fstream.cpp
  xml/parse/util/valid_name.cpp
  xml/parse/util/cdata.cpp
  xml/parse/util/expect.cpp
  xml/parse/util/name_element.cpp
  xml/parse/util/optional.cpp
  xml/parse/util/required.cpp
  xml/parse/util/skip.cpp
  xml/parse/util/validprefix.cpp
  xml/parse/util/validstructfield.cpp
  xml/parse/util/property.cpp
  xml/parse/util/show_node_type.cpp
  xml/parse/util/weparse.cpp
  xml/parse/util/position.cpp
  xml/parse/warning.cpp
)

target_link_libraries (xml
  we
  fhg-util
  ${Boost_LIBRARIES}
)

get_target_property(GEN_INCLUDE_EXE gen-includes LOCATION)

set (headerlist_hpp ${CMAKE_CURRENT_BINARY_DIR}/xml/parse/headerlist.hpp)

add_custom_command(OUTPUT ${headerlist_hpp}
  COMMAND ${GEN_INCLUDE_EXE} -o ${headerlist_hpp}
  DEPENDS gen-includes ${CMAKE_CURRENT_BINARY_DIR}/xml/parse/includes.hpp
  COMMENT "generating internal pnetc headerlist"
)

set_source_files_properties (
  "${CMAKE_CURRENT_SOURCE_DIR}/xml/parse/parser.cpp"
  PROPERTIES OBJECT_DEPENDS "${headerlist_hpp}"
)

if (BUILD_TESTING)
  add_subdirectory (tests)
  add_subdirectory (xml/tests)
endif()
