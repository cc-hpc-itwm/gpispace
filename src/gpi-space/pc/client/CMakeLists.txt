extended_add_library (NAME gpi-space-pc-client-static SOURCES "api.cpp"
  LIBRARIES Util::Generic
)
#! \todo move into extendend_add_library
target_include_directories (gpi-space-pc-client-static
  SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
)
