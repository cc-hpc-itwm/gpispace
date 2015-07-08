set (srcs
  segment.cpp
  #  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-segment ${srcs})
target_include_directories (gpi-space-pc-segment
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)

target_link_libraries (gpi-space-pc-segment ${LIBRT_STATIC_LIBRARY} Util::Generic)
