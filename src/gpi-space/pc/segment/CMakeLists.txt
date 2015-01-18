set (srcs
  segment.cpp
  #  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-segment ${srcs})
add_library(gpi-space-pc-segment-shared SHARED ${srcs})
target_include_directories (gpi-space-pc-segment
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
target_include_directories (gpi-space-pc-segment-shared
  PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
)
set_target_properties(gpi-space-pc-segment-shared PROPERTIES OUTPUT_NAME gpi-space-pc-segment)

target_link_libraries (gpi-space-pc-segment ${LIBRT_SHARED_LIBRARY})
target_link_libraries (gpi-space-pc-segment-shared ${LIBRT_SHARED_LIBRARY})
