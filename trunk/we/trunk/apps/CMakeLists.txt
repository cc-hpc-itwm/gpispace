# -*- mode: cmake; -*-

add_executable(we-execute "we-exec.cpp")
target_link_libraries(we-exec ${FhgLog_LIBRARY})
target_link_libraries(we-exec ${Boost_LIBRARIES})
target_link_libraries(we-exec dl)
