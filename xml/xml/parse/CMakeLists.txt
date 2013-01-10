# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (id)
add_subdirectory (rapidxml)
add_subdirectory (type)
add_subdirectory (util)

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

add_xml_sources (
  error.cpp
  parser.cpp
  state.cpp
  warning.cpp
)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/includes.hpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/includes.hpp
                @ONLY
               )

add_executable(gen-includes includes.cpp)
target_link_libraries(gen-includes fhg-revision)
target_link_libraries(gen-includes ${Boost_LIBRARIES})

add_executable(pnetc pnetc.cpp)
target_link_libraries(pnetc fhg-revision)
target_link_libraries (pnetc xml)
target_link_libraries(pnetc we)
target_link_libraries(pnetc  ${Boost_LIBRARIES})

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static pnetc.cpp ${headerlist_hpp})
  target_link_libraries(pnetc.static fhg-revision)
  target_link_libraries(pnetc.static we)
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})
  target_link_libraries (pnetc.static xml)
  target_link_libraries (pnetc.static fhg-revision)

  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static RUNTIME DESTINATION bin COMPONENT runtime)
endif (ENABLE_STATIC_BINARIES)

install (FILES ${hdrs} ${headerlist_hpp} ${fwd_hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)
