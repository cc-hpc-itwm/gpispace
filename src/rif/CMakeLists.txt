find_package (Libssh2 REQUIRED)

gspc_add_library (NAME gspc_rif_entry_point
  TYPE STATIC
  SOURCES "entry_point.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES PRIVATE gspc_util
            PRIVATE Boost::serialization
)

gspc_add_executable (NAME gspc-rifd
  RUNTIME
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES gspc_rpc
            gspc_util
            gspc_logging
            gspc_util
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
            PRIVATE gspc_rif_entry_point
            PRIVATE fmt
  CREATE_BUNDLE_INFO
)

gspc_add_library (NAME gspc_rif_started_process_promise
  # Object library for sake of re-exporting in libgspc.
  TYPE OBJECT
  SOURCES "started_process_promise.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES GPISpace::header-only
            Boost::iostreams
            Boost::program_options
            Boost::serialization
            gspc_util
            PRIVATE fmt
)

install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/rif/entry_point.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rif/entry_point.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rif/started_process_promise.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rif/started_process_promise.ipp"
  DESTINATION include/gspc/rif
)

gspc_add_library (NAME gspc_rif_strategies
  TYPE STATIC
  SOURCES "strategy/local.cpp"
          "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/ssh.cpp"
          "strategy/ssh/context.cpp"
          "strategy/ssh/detail.cpp"
          "strategy/ssh/session.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES
    PRIVATE gspc_rpc
    PRIVATE gspc_util
    PRIVATE Libssh2::libssh2
    PRIVATE OpenSSL::Crypto
    PRIVATE OpenSSL::SSL
    PRIVATE gspc_rif_entry_point
    PUBLIC Boost::program_options
    PUBLIC Boost::serialization
    PRIVATE Boost::thread
    PRIVATE fmt
)

gspc_add_executable (NAME gspc-bootstrap-rifd
  RUNTIME
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES gspc_rif_strategies
            gspc_configuration
            gspc_util
            gspc_util
            Boost::program_options
            Boost::system
            PRIVATE fmt
  CREATE_BUNDLE_INFO
)
gspc_add_executable (NAME gspc-teardown-rifd
  RUNTIME
  SOURCES "gspc-teardown-rifd.cpp"
  LIBRARIES gspc_rif_strategies
            gspc_configuration
            gspc_util
            gspc_util
            Boost::program_options
            Boost::system
            PRIVATE fmt
  CREATE_BUNDLE_INFO
)
