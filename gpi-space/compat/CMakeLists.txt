include_directories(${FVM_INCLUDE_DIR})

include_directories ("${CMAKE_SOURCE_DIR}/gspc")
include_directories ("${CMAKE_SOURCE_DIR}/src")

set (srcs
  pc-api.cpp
)

add_library (fvm-pc-compat ${srcs})
add_library (fvm-pc-compat.shared SHARED ${srcs})
set_target_properties(fvm-pc-compat.shared PROPERTIES OUTPUT_NAME fvm-pc-compat)

target_link_libraries (fvm-pc-compat
  gpi-space-pc-client-static
  gpi-space-pc-segment
  ${LibReadline_LIBRARY}
  ${FhgLog_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries (fvm-pc-compat.shared
  gpi-space-pc-client-static
  gpi-space-pc-segment

  # todo add shared libs for boost....
)
