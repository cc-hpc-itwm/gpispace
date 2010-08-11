# -*- mode: cmake; -*-

add_executable(fhgcomd fhgcomd.cpp)
target_link_libraries(fhgcomd ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS fhgcomd DESTINATION bin)
