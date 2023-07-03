gspc_add_executable (NAME drts-kernel
  RUNTIME
  # \note Exporting symbols so that modules see e.g. API guard symbol
  # and don't need to link explicitly against pnet or we.
  ENABLE_EXPORTS
  SOURCES "drts-kernel.cpp"
          "drts.cpp"
  LIBRARIES GPISpace::APIGuard
            GPISpace::header-only
            fhg-util
            fhgcom
            sdpa
            we-dev
            $<$<BOOL:${GSPC_WITH_IML}>:IML::Client>
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

gspc_add_library (NAME drts-context
  SOURCES "context.cpp"
  LIBRARIES GPISpace::header-only
            gspc::logging
            Boost::iostreams
  TYPE SHARED
  INSTALL
  CREATE_BUNDLE_INFO
)
