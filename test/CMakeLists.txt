set (_default_test_certs "${CMAKE_CURRENT_BINARY_DIR}/certs")
set (GSPC_SSL_CERTIFICATES_FOR_TESTS "${_default_test_certs}"
  CACHE STRING "SSL certificates to use in tests"
)

message (STATUS "Using '${GSPC_SSL_CERTIFICATES_FOR_TESTS}' for tests using SSL")

set (_test_rsa_bits 1024)
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
  COMMAND openssl dhparam -out "${_default_test_certs}/dh2048.pem" ${_test_dh_bits}
)

add_custom_target (test-certificates
  DEPENDS "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/dh2048.pem"
          "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/server.crt"
          "${GSPC_SSL_CERTIFICATES_FOR_TESTS}/server.key"
)

extended_add_library (NAME test-utilities
  SOURCES "certificates_data.cpp"
          "hopefully_free_port.cpp"
          "make.cpp"
          "scoped_nodefile_from_environment.cpp"
          "shared_directory.cpp"
          "source_directory.cpp"
          "virtual_memory_socket_name_for_localhost.cpp"
  INCLUDE_DIRECTORIES PUBLIC "${CMAKE_SOURCE_DIR}"
  LIBRARIES fhg-util
            Boost::program_options
            Boost::filesystem
            Util::Generic
            RPC
  COMPILE_DEFINITIONS GSPC_SSL_CERTIFICATES_FOR_TESTS="${GSPC_SSL_CERTIFICATES_FOR_TESTS}"
  DEPENDS test-certificates
)
