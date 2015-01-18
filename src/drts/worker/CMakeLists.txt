# bernd.loerwald@itwm.fraunhofer.de

add_library (fake-drts-kernel-plugin drts.cpp)
target_include_directories (fake-drts-kernel-plugin
  PRIVATE "${HWLOC_INCLUDE_DIR}"
)
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
  LINK_LIBRARIES fake-drts-kernel-plugin
                 ${UTIL_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_IOSTREAMS_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
)
target_include_directories (drts-kernel
  PRIVATE "${HWLOC_INCLUDE_DIR}"
)

install (FILES context_fwd.hpp context.hpp

  DESTINATION include/drts/worker
)
