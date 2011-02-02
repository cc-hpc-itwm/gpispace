set (hdrs
  )

set (srcs
  gpi-space.cpp
  )

add_executable (gpi-space ${srcs})
target_link_libraries (gpi-space
  gpi-space-gpiapi
  gpi-space-static
  ${MMGR_LIBRARY}
  ${FhgLog_LIBRARY}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(gpi-space
  PROPERTIES
  INSTALL_RPATH ""
  )
install (TARGETS gpi-space
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
