set (hdrs
  error.hpp
#  pnetc.hpp
  state.hpp
  types.hpp
  util.hpp
  warning.hpp
  )

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/includes.hpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/includes.hpp
                @ONLY
               )

add_executable(gen-includes includes.cpp)
target_link_libraries(gen-includes ${Boost_LIBRARIES})

set (headerlist_hpp
    ${CMAKE_CURRENT_BINARY_DIR}/headerlist.hpp
    )
set (headerlist_gen
     gen-includes
    )
add_custom_command(OUTPUT ${headerlist_hpp}
                   COMMAND ${headerlist_gen} > ${headerlist_hpp}
                   DEPENDS ${headerlist_gen}
                  )

add_executable(pnetc pnetc.cpp ${headerlist_hpp})
target_link_libraries(pnetc ${Boost_LIBRARIES})
target_link_libraries(pnetc ${WE_PRECOMPILE_LIBRARIES})
install (TARGETS pnetc
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static pnetc.cpp ${headerlist_hpp})
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})
  target_link_libraries(pnetc.static ${WE_PRECOMPILE_LIBRARIES})
  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static
    RUNTIME
    DESTINATION bin
    COMPONENT applications)
endif (ENABLE_STATIC_BINARIES)

install (FILES ${hdrs}
  DESTINATION include/xml/parse
  COMPONENT headers)

add_subdirectory (type)
add_subdirectory (util)
