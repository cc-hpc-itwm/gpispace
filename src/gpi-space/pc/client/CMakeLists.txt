set (srcs
  api.cpp
  )

add_library(gpi-space-pc-client-static ${srcs})
add_library(gpi-space-pc-client-shared SHARED ${srcs})
set_target_properties(gpi-space-pc-client-static PROPERTIES OUTPUT_NAME gpi-space-pc-client)
set_target_properties(gpi-space-pc-client-shared PROPERTIES OUTPUT_NAME gpi-space-pc-client)
