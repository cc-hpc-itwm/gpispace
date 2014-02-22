set (hdrs
  segment.hpp
  #  manager.hpp
  )

set (srcs
  segment.cpp
  #  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-segment ${hdrs} ${srcs})
add_library(gpi-space-pc-segment-shared SHARED ${hdrs} ${srcs})
set_target_properties(gpi-space-pc-segment-shared PROPERTIES OUTPUT_NAME gpi-space-pc-segment)

target_link_libraries (gpi-space-pc-segment ${LIBRT_SHARED_LIBRARY})
target_link_libraries (gpi-space-pc-segment-shared ${LIBRT_SHARED_LIBRARY})
