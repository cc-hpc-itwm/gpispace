# -*- mode: cmake; -*-

add_executable(logtest logtest.cpp)
target_link_libraries(logtest fhglog)

add_executable(test_append test_append.cpp)
target_link_libraries(test_append fhglog)

get_target_property(LOC logtest LOCATION_)
add_test(logtest ${LOC})

get_target_property(LOG test_append LOCATION_)
add_test(test_append ${LOC})
