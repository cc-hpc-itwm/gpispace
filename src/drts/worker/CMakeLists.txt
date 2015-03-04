# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (drts-kernel.cpp drts.cpp
  LINK_LIBRARIES fhg-util
                 fhgcom
                 sdpa
                 we-dev
                 rif-startup-messages-pipe
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${HWLOC_SHARED_LIBRARY}
)
target_include_directories (drts-kernel
  PRIVATE "${HWLOC_INCLUDE_DIR}"
)

install (FILES context_fwd.hpp context.hpp
  DESTINATION include/drts/worker
)

add_library (drts-context SHARED context.cpp)
target_link_libraries (drts-context fhglog)

install (TARGETS drts-context
  LIBRARY DESTINATION lib
)
