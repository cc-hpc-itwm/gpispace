add_library (gpi-space-pc-container
  manager.cpp
)
target_include_directories (gpi-space-pc-container
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
