find_package(libssh2 QUIET REQUIRED COMPONENTS _relocatable)

#! \note need to use openssl API to ensure threadsafety, if ssh2 uses it
if (TARGET libssh2-ssl)
  find_package (OpenSSL QUIET REQUIRED)

  get_filename_component (_openssl-found "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
  get_property (_openssl-ssh2-lib TARGET libssh2-ssl PROPERTY IMPORTED_LOCATION)
  get_filename_component (_openssl-ssh2-libdir "${_openssl-ssh2-lib}" DIRECTORY)
  get_filename_component (_openssl-ssh2 "${_openssl-ssh2-libdir}" DIRECTORY)

  if (NOT "${_openssl-found}" STREQUAL "${_openssl-ssh2}")
    message (FATAL_ERROR "libssh2 uses openssl [${_openssl-ssh2}] but"
      " find_package found a different openssl [${_openssl-found}]"
    )
  endif()
endif()

fhg_add_runtime_executable (NAME gspc-rifd
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES RPC
            fhg-util
            gspc::logging
            GPISpace::vmem
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
  CREATE_BUNDLE_INFO
)

extended_add_library (NAME rif-started_process_promise
  SOURCES "started_process_promise.cpp"
  LIBRARIES Boost::iostreams
            Boost::program_options
            Boost::serialization
            Util-Generic
  INSTALL
)

install (FILES
  started_process_promise.hpp
  started_process_promise.ipp
  DESTINATION include/rif
)

add_unit_test (NAME started_process_promise
  SOURCES "test/started_process_promise.cpp"

  DESCRIPTION "pipe used for communicating with a rif-started process during startup"
  USE_BOOST
  LIBRARIES rif-started_process_promise
            Util-Generic
)

extended_add_library (NAME rif-strategies
  SOURCES "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/srun.cpp"
          "strategy/ssh.cpp"
          "strategy/ssh/context.cpp"
          "strategy/ssh/detail.cpp"
          "strategy/ssh/session.cpp"
  LIBRARIES RPC
            fhg-util
            libssh2
)

fhg_add_runtime_executable (NAME gspc-bootstrap-rifd
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-revision
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
fhg_add_runtime_executable (NAME gspc-teardown-rifd
  SOURCES "gspc-teardown-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-revision
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
