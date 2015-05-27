extended_add_library (NAME gpi-space-gpiapi
  SOURCES "system.cpp"
          "fake_api.cpp"
          "gaspi.cpp"
  LIBRARIES ${GASPI_LIBRARY}
            ${IBverbs_LIBRARY}
            Util::Generic
)
#! \todo move into extended_add_library
target_include_directories (gpi-space-gpiapi
  PRIVATE "${GASPI_INCLUDE_DIR}"
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
