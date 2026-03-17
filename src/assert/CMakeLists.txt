add_library (gspc_assert INTERFACE)
target_include_directories (gspc_assert
  INTERFACE "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries (gspc_assert INTERFACE fmt)
