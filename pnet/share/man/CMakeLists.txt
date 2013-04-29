configure_file(xpnet.5.in xpnet.5 @ONLY)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/xpnet.5"
  DESTINATION share/sdpa/man/man5
  COMPONENT sdk
)
