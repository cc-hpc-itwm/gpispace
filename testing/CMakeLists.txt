if (BUILD_TESTING)
  set (_default_test_certs "${CMAKE_CURRENT_BINARY_DIR}/certs")
  set (GSPC_SSL_CERTIFICATES_FOR_TESTS "${_default_test_certs}"
    CACHE STRING "SSL certificates to use in tests"
  )

  message (STATUS "Using '${GSPC_SSL_CERTIFICATES_FOR_TESTS}' for tests using SSL")

  set (_test_rsa_bits 2048)
  set (_test_dh_bits 2048)

  add_custom_command (OUTPUT "${_default_test_certs}/server.key"
                             "${_default_test_certs}/server.crt"
    COMMAND mkdir -p "${_default_test_certs}/"
    COMMAND openssl genrsa -des3
                           -passout pass:pass
                           -out "${_default_test_certs}/server.key.secure"
                           ${_test_rsa_bits}
    COMMAND openssl rsa -passin pass:pass
                        -in "${_default_test_certs}/server.key.secure"
                        -out "${_default_test_certs}/server.key"
    COMMAND openssl req -new
                        -key "${_default_test_certs}/server.key"
                        -out "${_default_test_certs}/server.csr"
                        -batch
    COMMAND openssl x509 -req
                         -days 365
                         -in "${_default_test_certs}/server.csr"
                         -signkey "${_default_test_certs}/server.key"
                         -out "${_default_test_certs}/server.crt"
    COMMAND rm "${_default_test_certs}/server.csr"
               "${_default_test_certs}/server.key.secure"
  )
  add_custom_command (OUTPUT "${_default_test_certs}/dh2048.pem"
    COMMAND mkdir -p "${_default_test_certs}/"
    # -dsaparam is weak against subgroup attacks when reused, so one
    # should only use it once per key exchange, which GPI-Space
    # doesn't. In tests security isn't the biggest concern though (and
    # can't as the data is user provided and not under GPI-Space
    # control), so not spending excessive time on generating a strong
    # prime is a fair trade-off.
    # Do *not* use "-dsaparam" in a production environment!
    COMMAND openssl dhparam -dsaparam -out "${_default_test_certs}/dh2048.pem" ${_test_dh_bits}
  )

  add_custom_target (test-certificates
    DEPENDS "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/dh2048.pem"
            "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/server.crt"
            "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/server.key"
  )

  configure_file ("make_environment.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/make_environment.hpp"
    @ONLY ESCAPE_QUOTES
  )

  gspc_add_library (NAME test-util-certificates
    TYPE STATIC
    SOURCES "certificates_data.cpp"
    LIBRARIES Boost::base Boost::filesystem
              PUBLIC GPISpace-header-only
              PRIVATE fmt
              PRIVATE Util::Generic
    INCLUDE_DIRECTORIES PUBLIC "${CMAKE_SOURCE_DIR}"
    COMPILE_DEFINITIONS GSPC_SSL_CERTIFICATES_FOR_TESTS="${GSPC_SSL_CERTIFICATES_FOR_TESTS}"
    DEPENDS test-certificates
  )

  gspc_add_library (NAME test-utilities
    TYPE STATIC
    SOURCES "hopefully_free_port.cpp"
            "make.cpp"
            "parse_command_line.cpp"
            "scoped_nodefile_from_environment.cpp"
            "shared_directory.cpp"
            "source_directory.cpp"
            "fmt_directory.cpp"
            "virtual_memory_socket_name_for_localhost.cpp"
    INCLUDE_DIRECTORIES PUBLIC "${CMAKE_SOURCE_DIR}"
                        PRIVATE "${CMAKE_BINARY_DIR}"
    LIBRARIES gspc
              test-util-certificates
              fhg-util
              Boost::program_options
              Boost::filesystem
              Util::Generic
              Util::RPC
    COMPILE_DEFINITIONS
      PUBLIC
        GSPC_WITH_IML=$<BOOL:${GSPC_WITH_IML}>
  )
endif()
