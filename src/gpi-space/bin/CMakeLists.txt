add_executable (gpi-space gpi-space.cpp)
target_link_libraries (gpi-space
  gpi-space-gpiapi
  gpi-space-pc-container
  gpi-space-pc-global
  gpi-space-pc-memory
  gpi-space-pc-segment
  fhg-revision
  fhg-util
  ${MMGR_LIBRARY}
  ${FhgCom_LIBRARY}
  ${FhgLog_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

set_target_properties(gpi-space
  PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
  )
install (TARGETS gpi-space
  RUNTIME
  DESTINATION bin
)
