set (srcs
  api.cpp
  )

add_library(gpi-space-pc-client-static ${srcs})
target_include_directories (gpi-space-pc-client-static
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
target_link_libraries (gpi-space-pc-client-static Util::Generic)
set_target_properties(gpi-space-pc-client-static PROPERTIES OUTPUT_NAME gpi-space-pc-client)
