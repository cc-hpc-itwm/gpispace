string (TIMESTAMP COPYRIGHT_YEAR "%Y")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/configuration.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/configuration.cpp"
  @ONLY
)

gspc_add_library (NAME gspc_configuration
  TYPE STATIC
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/configuration.cpp"
  INCLUDE_DIRECTORIES PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
