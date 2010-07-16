add_executable(parser parser.cpp)
target_link_libraries(parser ${Boost_LIBRARIES})
install (TARGETS parser DESTINATION bin)
