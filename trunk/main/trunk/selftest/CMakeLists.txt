configure_file(${CMAKE_CURRENT_SOURCE_DIR}/selftest.sh.in ${CMAKE_CURRENT_BINARY_DIR}/selftest.sh @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/selftest.sh
  DESTINATION bin
  COMPONENT applications
  RENAME sdpa-selftest)

install(FILES selftest.pnet
  DESTINATION share/sdpa/examples/selftest/
  COMPONENT examples
  )
