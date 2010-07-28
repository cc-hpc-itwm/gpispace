add_executable(pnetc parser.cpp)
target_link_libraries(pnetc ${Boost_LIBRARIES})
install (TARGETS pnetc DESTINATION bin)

add_subdirectory (util)
