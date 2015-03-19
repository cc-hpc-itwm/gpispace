add_library (gpi-space-gpiapi
  system.cpp
  fake_api.cpp
  gaspi.cpp
)
target_include_directories (gpi-space-gpiapi
  PRIVATE "${GASPI_INCLUDE_DIR}"
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
target_link_libraries (gpi-space-gpiapi ${GASPI_LIBRARY} ${IBverbs_LIBRARY} Util::Generic)
