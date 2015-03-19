ADD_LIBRARY(gpi-space-pc-global topology.cpp)
target_include_directories (gpi-space-pc-global
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
target_link_libraries(gpi-space-pc-global
  fhgcom
  Util::Generic
)
