# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (drts-kernel.cpp drts.cpp
  LINK_LIBRARIES fhg-util
                 fhgcom
                 sdpa
                 we-dev
                 rif-startup-messages-pipe
                 Util::Generic
                 Boost::filesystem
                 Boost::program_options
                 Boost::serialization
                 Boost::system
                 Boost::thread
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
