add_executable(pspromig-example main.cpp)
target_link_libraries(pspromig-example
  ${PSPROMIGIF_LIBRARY}
  ${HWLOC_LIBRARY_SHARED}
  ${LIBIRC_LIBRARY}
  pthread
  )
