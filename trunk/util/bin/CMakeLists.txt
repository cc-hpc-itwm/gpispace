add_executable(fhgcfg fhgcfg.cpp)
target_link_libraries(fhgcfg ${Boost_LIBRARIES})

add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

add_executable(fhgkernel fhgkernel.cpp)
target_link_libraries(fhgkernel fhg-plugin)

# this is required to be able to load modules that require parts of boost
# the binary doesn't use itself (like threads and stuff)
target_link_libraries(fhgmodinfo
  "-Wl,-whole-archive"
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  "-Wl,-no-whole-archive"
  )

target_link_libraries(fhgkernel
  "-Wl,-whole-archive"
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  "-Wl,-no-whole-archive"
  "-Wl,-E"
  )

install (TARGETS fhgcfg fhgmodinfo
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
