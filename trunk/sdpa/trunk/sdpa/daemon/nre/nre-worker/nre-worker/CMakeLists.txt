
add_executable(nre-pcd nre-pcd.cpp ActivityExecutor.cpp)
target_link_libraries(nre-pcd ${FhgLog_LIBRARY})
target_link_libraries(nre-pcd ${Boost_LIBRARIES})
target_link_libraries(nre-pcd dl)
target_link_libraries(nre-pcd pthread)

install(TARGETS nre-pcd
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
