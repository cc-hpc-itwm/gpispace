extended_add_library (NAME gpi-space-pc-container
  SOURCES "manager.cpp"
  LIBRARIES Util::Generic
)
#! \todo move into extended_add_library
target_include_directories (gpi-space-pc-container
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
