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
  ${gpi_space_Boost_FILESYSTEM_LIBRARY}
  ${gpi_space_Boost_SERIALIZATION_LIBRARY}
  ${gpi_space_Boost_THREAD_LIBRARY}
  ${gpi_space_Boost_SYSTEM_LIBRARY}
)

# search for libreadline
find_package (LibReadline REQUIRED QUIET)
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
  ${gpi_space_Boost_PROGRAM_OPTIONS_LIBRARY}
  ${gpi_space_Boost_SERIALIZATION_LIBRARY}
  ${gpi_space_Boost_THREAD_LIBRARY}
  ${gpi_space_Boost_FILESYSTEM_LIBRARY}
  ${gpi_space_Boost_SYSTEM_LIBRARY}
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
