find_package (FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

include_directories ("${CMAKE_SOURCE_DIR}/pnet/src")

set (srcs
  pc-api.cpp
)

add_library (fvm-pc-compat ${srcs})
add_library (fvm-pc-compat.shared SHARED ${srcs})
set_target_properties(fvm-pc-compat.shared PROPERTIES OUTPUT_NAME fvm-pc-compat)

target_link_libraries (fvm-pc-compat
  gpi-space-pc-client-static
  gpi-space-pc-segment
  gpi-space-static
  ${LibReadline_LIBRARY}
  ${FhgLog_LIBRARY}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries (fvm-pc-compat.shared
  gpi-space-pc-client-static
  gpi-space-pc-segment
  gpi-space-static

  # todo add shared libs for boost....
)

#install(TARGETS fvm-pc-compat
#  ARCHIVE
#  DESTINATION libexec
#  COMPONENT modules)
#install(TARGETS fvm-pc-compat.shared
#  LIBRARY
#  DESTINATION libexec
#  COMPONENT modules)
