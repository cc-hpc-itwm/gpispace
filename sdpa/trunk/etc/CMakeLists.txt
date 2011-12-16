configure_file(sdpa.env.in ${CMAKE_CURRENT_BINARY_DIR}/sdpa.env.template @ONLY)

INSTALL(FILES defaults.template
  DESTINATION etc/sdpa
  COMPONENT configurations)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdpa.env.template
  DESTINATION etc/sdpa
  COMPONENT configurations)
