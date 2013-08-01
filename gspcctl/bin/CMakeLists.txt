add_executable (gspc gspc.cpp)
target_link_libraries (gspc
  gspcctl
  ${UTIL_LIBRARIES}
  ${GSPC_RIF_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

# TODO: activate when full replacement for 'gspc' script is working
#install (TARGETS gspc
#  DESTINATION bin
#  COMPONENT runtime
#)
