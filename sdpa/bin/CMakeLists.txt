add_subdirectory(pbs)
add_subdirectory(monos)

add_executable (drts-kernel drts-kernel.cpp)
target_link_libraries (drts-kernel fhg-plugin we-dev ${UTIL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_THREAD_LIBRARY})

install (TARGETS drts-kernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
