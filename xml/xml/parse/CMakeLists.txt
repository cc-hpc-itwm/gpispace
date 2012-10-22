# bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  error.hpp
  headergen.hpp
  parser.hpp
  state.hpp
  type_map_type.hpp
  util.hpp
  warning.hpp
  )

set (fwd_hdrs
  state.fwd.hpp
)

set (sources
  state.cpp
)

add_library (xml_parse
  ${sources}
)

target_link_libraries(xml_parse  ${Boost_LIBRARIES})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/includes.hpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/includes.hpp
                @ONLY
               )

add_executable(gen-includes includes.cpp)
target_link_libraries(gen-includes ${Boost_LIBRARIES})
target_link_libraries(gen-includes fhg-revision)
get_target_property(GEN_INCLUDE_EXE gen-includes LOCATION)

set (headerlist_hpp
    ${CMAKE_CURRENT_BINARY_DIR}/headerlist.hpp
    )
add_custom_command(OUTPUT ${headerlist_hpp}
  COMMAND ${GEN_INCLUDE_EXE} -o ${headerlist_hpp}
  DEPENDS gen-includes
  COMMENT "generating internal pnetc headerlist"
  )

add_custom_target(gen_headerlist ALL
  DEPENDS ${headerlist_hpp}
  COMMENT "generating internal pnetc/pnete headerlist"
  )
set_source_files_properties(pnetc.cpp OBJECT_DEPENDS ${headerlist_hpp})

add_executable(pnetc pnetc.cpp)
#add_dependencies(pnetc gen-includes)
target_link_libraries(pnetc fhg-revision)
target_link_libraries(pnetc ${WE_LIBRARIES})
target_link_libraries(pnetc  ${Boost_LIBRARIES})
target_link_libraries (pnetc xml_parse_type)
target_link_libraries (pnetc xml_parse_util)
target_link_libraries (pnetc xml_parse)

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static pnetc.cpp ${headerlist_hpp})
  target_link_libraries(pnetc.static fhg-revision)
  target_link_libraries(pnetc.static ${WE_LIBRARIES})
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})
  target_link_libraries (pnetc.static xml_parse_type)
  target_link_libraries (pnetc.static xml_parse_util)
  target_link_libraries (pnetc.static xml_parse)

  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static RUNTIME DESTINATION bin COMPONENT runtime)
endif (ENABLE_STATIC_BINARIES)

install (FILES ${hdrs} ${headerlist_hpp} ${fwd_hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)

add_subdirectory (type)
add_subdirectory (util)
add_subdirectory (rapidxml)

