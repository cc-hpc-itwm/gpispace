fhg_add_test (NAME vmem_url
  SOURCES url.cpp
  USE_BOOST
  INCLUDE SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES gpi-space_url
)
