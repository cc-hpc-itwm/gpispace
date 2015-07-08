extended_add_library (NAME gpi-space-gpiapi
  SOURCES "system.cpp"
          "fake_api.cpp"
          "gaspi.cpp"
  LIBRARIES GASPI::all
            Util::Generic
)
#! \todo move into extended_add_library
target_include_directories (gpi-space-gpiapi
  PRIVATE "${GASPI_INCLUDE_DIR}"
)
