# bernd.loerwald@itwm.fraunhofer.de

find_package (HWLOC REQUIRED QUIET)
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

add_library (fake-drts-kernel-plugin drts.cpp job.cpp)
target_link_libraries (fake-drts-kernel-plugin
  we-dev
  ${UTIL_LIBRARIES}
  sdpa
  ${FhgCom_LIBRARY}
  ${GSPC_LIBRARY_SHARED}
  ${HWLOC_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

add_executable (drts-kernel drts-kernel.cpp)
target_link_libraries (drts-kernel
  fhg-plugin
  fake-drts-kernel-plugin
  ${UTIL_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY})

install (TARGETS drts-kernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
