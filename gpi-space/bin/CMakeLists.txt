set (hdrs
  )

set (srcs
  gpi-space.cpp
  )

add_executable (gpi-space ${srcs})
target_link_libraries (gpi-space
  gpi-space-gpiapi
  gpi-space-pc-server
  ${CMAKE_THREAD_LIBS_INIT}
  fhg-revision
)

# search for libreadline
find_package (LibReadline REQUIRED)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

set (srcs
  gpish.cpp
  )

add_executable (gpish ${srcs})
target_link_libraries (gpish
  gpi-space-pc-client-static
  gpi-space-pc-segment
  gpi-space-static
  ${LibReadline_LIBRARY}
  ${FhgLog_LIBRARY}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  fhg-revision
)

set_target_properties(gpi-space
  PROPERTIES
  INSTALL_RPATH ""
  )
install (TARGETS gpi-space gpish
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
