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
set_target_properties(pnetc PROPERTIES LINK_FLAGS "-static")
install (TARGETS pnetc
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

install (FILES ${hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)

add_subdirectory (type)
add_subdirectory (util)
