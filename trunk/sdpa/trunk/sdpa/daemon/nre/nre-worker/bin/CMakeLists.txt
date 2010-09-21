configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run-pcd.real.in ${CMAKE_CURRENT_BINARY_DIR}/run-pcd.real)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run-pcd.fake.in ${CMAKE_CURRENT_BINARY_DIR}/run-pcd.fake)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run-pcd.real ${CMAKE_CURRENT_BINARY_DIR}/run-pcd.fake
  DESTINATION bin
  COMPONENT applications)
