# bernd.loerwald@itwm.fraunhofer.de

find_package (HWLOC REQUIRED QUIET)
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

add_library (fake-drts-kernel-plugin drts.cpp job.cpp)
target_link_libraries (fake-drts-kernel-plugin
  we-dev
  ${UTIL_LIBRARIES}
  sdpa
  ${FhgCom_LIBRARY}
  ${HWLOC_SHARED_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

fhg_add_runtime_executable (drts-kernel.cpp
  LINK_LIBRARIES fhg-plugin
                 fake-drts-kernel-plugin
                 ${UTIL_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
)

install (FILES context_fwd.hpp context.hpp

  DESTINATION include/drts/worker
)
