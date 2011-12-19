# -*- mode: cmake; -*-

add_executable(fhgcomd fhgcomd.cpp)
target_link_libraries(fhgcomd fhgcom ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

add_executable(fhgcomc fhgcomc.cpp)
target_link_libraries(fhgcomc fhgcom ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

add_executable(fhgkvsd fhgkvsd.cpp)
target_link_libraries(fhgkvsd fhgcom ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

add_executable(fhgkvsc fhgkvsc.cpp)
target_link_libraries(fhgkvsc fhgcom ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

add_executable(fhgaddr fhgaddr.cpp)
target_link_libraries(fhgaddr fhgcom ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS fhgkvsd
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)

install(TARGETS fhgkvsc
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)

install(TARGETS fhgaddr
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
