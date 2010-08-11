# -*- mode: cmake; -*-

add_executable(fhgcomd fhgcomd.cpp)
target_link_libraries(fhgcomd ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

add_executable(fhgcomc fhgcomc.cpp)
target_link_libraries(fhgcomc ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS fhgcomd DESTINATION bin)
install(TARGETS fhgcomc DESTINATION bin)
