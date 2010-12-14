set (hdrs
  error.hpp
  parser.hpp
  state.hpp
  types.hpp
  util.hpp
  warning.hpp
  )

add_executable(pnetc parser.cpp)
target_link_libraries(pnetc ${Boost_LIBRARIES})

add_executable(pnetc.static parser.cpp)
target_link_libraries(pnetc.static ${Boost_LIBRARIES})
set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

install (TARGETS pnetc pnetc.static
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

install (FILES ${hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)

add_subdirectory (type)
add_subdirectory (util)
