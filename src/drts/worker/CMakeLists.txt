# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME drts-kernel
  SOURCES "drts-kernel.cpp"
          "drts.cpp"
          "../cache_management/cache_manager.cpp"
          "../cache_management/circular_cache_manager.cpp"
  LIBRARIES fhg-util
            fhgcom
            sdpa
            we-dev
            rif-started_process_promise
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
            Boost::thread
            HWLOC::static
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

install (FILES context_fwd.hpp context.hpp
  DESTINATION include/drts/worker
)

extended_add_library (NAME drts-context
  SOURCES "context.cpp"
  LIBRARIES fhglog
            Boost::iostreams
  TYPE SHARED
  INSTALL
)
