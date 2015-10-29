extended_add_library (NAME gpi-space-gpiapi
  SOURCES "system.cpp"
          "gaspi.cpp"
  LIBRARIES GASPI::static
            Util::Generic
)
#! \todo move into extended_add_library
target_include_directories (gpi-space-gpiapi
  PRIVATE "${GASPI_INCLUDE_DIR}"
  SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
)
