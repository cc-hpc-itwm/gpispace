add_executable(pnetc parser.cpp)
target_link_libraries(pnetc ${Boost_LIBRARIES})
set_target_properties(pnetc PROPERTIES LINK_FLAGS "-static")
install (TARGETS pnetc DESTINATION bin)

add_subdirectory (util)
