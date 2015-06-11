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
                 HWLOC::static
)

install (FILES context_fwd.hpp context.hpp
  DESTINATION include/drts/worker
)

extended_add_library (NAME drts-context
  SOURCES "context.cpp"
  LIBRARIES fhglog
  TYPE SHARED
  INSTALL
)
