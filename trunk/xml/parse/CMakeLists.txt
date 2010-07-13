add_executable(parser parser.cpp)
target_link_libraries(parser ${Boost_LIBRARIES})
install (TARGETS parser DESTINATION bin)

add_executable(toDot toDot.cpp)
target_link_libraries(toDot ${Boost_LIBRARIES})
install (TARGETS toDot DESTINATION bin)
