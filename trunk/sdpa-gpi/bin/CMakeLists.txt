#set (hdrs
#  )
#
#set (srcs
#  sdpa-gpi.cpp
#  )
#
#add_executable (sdpa-gpi-new sdpa-gpi.cpp)
#target_link_libraries (sdpa-gpi-new
#  ${GPI_LIBRARIES}
#  ${MMGR_LIBRARY}
#  ${FhgLog_LIBRARY}
#  ${Boost_LIBRARIES}
#  )
#
#set_target_properties(sdpa-gpi-new
#  PROPERTIES
#  INSTALL_RPATH ""
#  )
#install (TARGETS sdpa-gpi-new
#  RUNTIME
#  DESTINATION bin
#  COMPONENT applications)
