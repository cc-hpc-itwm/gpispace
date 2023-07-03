extended_add_library (NAME Headers
  NAMESPACE IMLPrivate
  INCLUDE_DIRECTORIES INTERFACE ${PROJECT_SOURCE_DIR}/src
)
target_compile_features (IMLPrivate-Headers
  INTERFACE "${_project_cxx_std}"
)

iml_add_library (
  NAME Shared
  NAMESPACE IMLPrivate
  SOURCES "vmem/gaspi/pc/segment/segment.cpp"
  LIBRARIES
    PUBLIC IMLPrivate::Headers
    PRIVATE Util::Generic
    PUBLIC Boost::base
    PRIVATE Boost::system
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  POSITION_INDEPENDENT
)

iml_add_library (
  NAME Client
  NAMESPACE IML
  TYPE SHARED
  VISIBILITY_HIDDEN
  SOURCES # API files
          "AllocationHandle.cpp"
          "Client.cpp"
          "MemoryLocation.cpp"
          "MemoryRegion.cpp"
          "Rifs.cpp"
          "RuntimeSystem.cpp"
          "SegmentAndAllocation.cpp"
          "SegmentHandle.cpp"
          "SharedMemoryAllocation.cpp"
          "SharedMemoryAllocationHandle.cpp"
          "beegfs/SegmentDescription.cpp"
          "gaspi/NetdevID.cpp"
          "gaspi/SegmentDescription.cpp"
          "rif/EntryPoint.cpp"
          "rif/EntryPoints.cpp"
          "rif/bootstrap.cpp"
          "rif/strategies.cpp"
          "rif/teardown.cpp"

          # non-API files
          "detail/startup_and_shutdown.cpp"
          "rif/strategy/local.cpp"
          "rif/strategy/meta.cpp"
          "rif/strategy/pbsdsh.cpp"
          "rif/strategy/ssh.cpp"
          "rif/strategy/ssh/context.cpp"
          "rif/strategy/ssh/detail.cpp"
          "rif/strategy/ssh/session.cpp"
          "rif/strategy/system_with_blocked_SIGCHLD.cpp"
  LIBRARIES
    PRIVATE IMLPrivate::Headers
    PRIVATE IMLPrivate::Installation
    PRIVATE IMLPrivate::Shared
    PRIVATE Util::RPC
    PRIVATE Util::Generic
    PRIVATE Libssh2::libssh2
    PRIVATE OpenSSL::Crypto
    PRIVATE OpenSSL::SSL
    PUBLIC Boost::filesystem
    PUBLIC Boost::program_options
    PUBLIC Boost::serialization
    PRIVATE Boost::thread
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  INSTALL
    INSTALL_DESTINATION "lib"
    RPATH "libexec/iml"
)

iml_add_library (
  NAME Server
  NAMESPACE IMLPrivate
  SOURCES "vmem/dtmmgr.cpp"
          "vmem/gaspi/gpi/gaspi.cpp"
          "vmem/gaspi/pc/container/manager.cpp"
          "vmem/gaspi/pc/global/topology.cpp"
          "vmem/gaspi/pc/memory/beegfs_area.cpp"
          "vmem/gaspi/pc/memory/gaspi_area.cpp"
          "vmem/gaspi/pc/memory/handle_generator.cpp"
          "vmem/gaspi/pc/memory/manager.cpp"
          "vmem/gaspi/pc/memory/memory_area.cpp"
          "vmem/gaspi/pc/memory/shm_area.cpp"
          "vmem/gaspi_context.cpp"
          "vmem/segment/beegfs.cpp"
          "vmem/tmmgr.cpp"
  LIBRARIES
    PUBLIC IML::Client
    PUBLIC IMLPrivate::Headers
    PUBLIC IMLPrivate::Shared
    PUBLIC Util::RPC
    PRIVATE Util::Generic
    PRIVATE BeeGFS::ClientDevel
    PRIVATE GASPI::static
    PUBLIC Boost::filesystem
    PUBLIC Boost::serialization
    PRIVATE Boost::system
    PUBLIC Boost::thread
)

iml_add_executable (
  NAME iml-gpi-server
  SOURCES "server/iml-gpi-server.cpp"
  LIBRARIES IMLPrivate::Headers
            IMLPrivate::Server
            IMLPrivate::rif-started_process_promise
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
    RPATH "lib"
)

iml_add_library (
  NAME Installation
  NAMESPACE IMLPrivate
  TYPE SHARED
  VISIBILITY_HIDDEN
  SOURCES "detail/Installation.cpp"
  LIBRARIES
    PUBLIC IMLPrivate::Headers
    PRIVATE Util::Generic
    PRIVATE Boost::filesystem
  COMPILE_DEFINITIONS
# The number of /.. to append to the path of the installed libary to
# get back up to the installation root:
#   install/libexec/iml/libIMLPrivate-Installation.so/../../../ = install/
    PRIVATE "-DIML_INSTALLATION_SENTINEL_SUBDIR_COUNT=3"
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
)

iml_add_executable (
  NAME iml-rifd
  SOURCES "rif/iml-rifd.cpp"
          "rif/execute_and_get_startup_messages.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Headers
            IMLPrivate::Installation
            Util::RPC
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
    RPATH "lib"
          "libexec/iml"
)

iml_add_library (
  NAME rif-started_process_promise
  NAMESPACE IMLPrivate
  SOURCES "rif/started_process_promise.cpp"
  LIBRARIES
    PUBLIC IMLPrivate::Headers
    PRIVATE Util::Generic
    PRIVATE Boost::iostreams
    PRIVATE Boost::serialization
)

iml_add_executable (
  NAME iml-bootstrap-rifd
  SOURCES "rif/iml-bootstrap-rifd.cpp"
  LIBRARIES IML::Client
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  INSTALL
    INSTALL_DESTINATION "bin"
    RPATH "lib"
)

iml_add_executable (
  NAME iml-teardown-rifd
  SOURCES "rif/iml-teardown-rifd.cpp"
  LIBRARIES IML::Client
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  INSTALL
    INSTALL_DESTINATION "bin"
    RPATH "lib"
)
