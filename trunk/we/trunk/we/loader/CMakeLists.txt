# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we/loader)

add_executable(loader loader.cpp)
target_link_libraries(loader ${FhgLog_LIBRARY})
target_link_libraries(loader ${Boost_LIBRARIES})
target_link_libraries(loader dl)
target_link_libraries(loader pthread)
install (TARGETS loader DESTINATION bin)
