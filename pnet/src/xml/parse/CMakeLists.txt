# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (id)
add_subdirectory (rapidxml)
add_subdirectory (type)
add_subdirectory (util)

set (hdrs
  error.hpp
  parser.hpp
  state.hpp
  type_map_type.hpp
  util.hpp
  warning.hpp
  )

set (fwd_hdrs
  state.fwd.hpp
)

add_executable(pnetc pnetc.cpp)
target_link_libraries(pnetc fhg-revision)
target_link_libraries(pnetc pnet)
target_link_libraries(pnetc ${Boost_LIBRARIES})

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static pnetc.cpp)
  target_link_libraries(pnetc.static fhg-revision)
  target_link_libraries(pnetc.static pnet)
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})

  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static RUNTIME DESTINATION bin COMPONENT runtime)
endif (ENABLE_STATIC_BINARIES)

install (FILES ${hdrs} ${fwd_hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)
