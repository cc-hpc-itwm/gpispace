add_library (gspc_scheduler_types INTERFACE)
target_include_directories (gspc_scheduler_types
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
