fhg_add_runtime_executable (NAME iml-gpi-server
  SOURCES "iml-gpi-server.cpp"
  LIBRARIES IML::vmem-gpiapi
            IML::vmem-pc-container
            IML::vmem-pc-global
            IML::vmem-pc-memory
            IML::vmem-pc-segment
            IML::fhg-revision
            IML::fhg-util
            IML::mmgr
            fhgcom
            IML::rif-started_process_promise
            Util::Generic
            ${CMAKE_THREAD_LIBS_INIT}
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/iml/"
)

