add_executable(sdpac main.cpp)
#set_target_properties(sdpac PROPERTIES LINK_FLAGS "-static")

target_link_libraries(sdpac sdpa-client)
target_link_libraries(sdpac sdpa-events)
target_link_libraries(sdpac sdpa-util)
target_link_libraries(sdpac sdpa-com)
target_link_libraries(sdpac sdpa)
target_link_libraries(sdpac seda)
target_link_libraries(sdpac ${Boost_LIBRARIES})

install(TARGETS sdpac
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

  
add_executable(bsdpac blocking_client.cpp)
target_link_libraries(bsdpac sdpa-client)
target_link_libraries(bsdpac sdpa-events)
target_link_libraries(bsdpac sdpa-util)
target_link_libraries(bsdpac sdpa-com)
target_link_libraries(bsdpac sdpa)
target_link_libraries(bsdpac seda)
target_link_libraries(bsdpac ${Boost_LIBRARIES})

install(TARGETS bsdpac
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
  