# -*- mode: cmake; -*-

add_executable(logtest logtest.cpp)
target_link_libraries(logtest fhglog)
get_target_property(LOC logtest LOCATION_)
add_test(logtest ${LOC})

add_executable(test_append test_append.cpp)
target_link_libraries(test_append fhglog)
get_target_property(LOC test_append LOCATION_)
add_test(test_append ${LOC})

add_executable(test_disabled_logging test_disabled_logging.cpp)
target_link_libraries(test_disabled_logging fhglog)
get_target_property(LOC test_disabled_logging LOCATION_)
add_test(test_disabled_logging ${LOC})

add_executable(test_formatter test_formatter.cpp)
target_link_libraries(test_formatter fhglog)
get_target_property(LOC test_formatter LOCATION_)
add_test(test_formatter ${LOC})
