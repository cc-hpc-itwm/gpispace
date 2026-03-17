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

configure_file ("include/gspc/testing/make_environment.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/gspc/testing/make_environment.hpp"
  @ONLY ESCAPE_QUOTES
)

set (_gspc_testing_sources
  "src/random.cpp"
  "src/random/char.cpp"
  "src/random/string.cpp"
  "src/unique_path.cpp"
  "src/certificates_data.cpp"
  "src/hopefully_free_port.cpp"
  "src/make.cpp"
  "src/parse_command_line.cpp"
  "src/scoped_nodefile_from_environment.cpp"
  "src/shared_directory.cpp"
  "src/source_directory.cpp"
  "src/fmt_directory.cpp"
  "src/virtual_memory_socket_name_for_localhost.cpp"
)

if (GSPC_WITH_IML)
  list (APPEND _gspc_testing_sources
    "iml/beegfs_directory.cpp"
    "iml/parse_command_line.cpp"
    "iml/set_nodefile_from_environment.cpp"
    "iml/virtual_memory_socket_name_for_localhost.cpp"
    "iml/random/AllocationHandle.cpp"
    "iml/random/MemoryLocation.cpp"
    "iml/random/MemoryRegion.cpp"
    "iml/random/SegmentHandle.cpp"
  )
endif()

add_library (gspc_testing STATIC ${_gspc_testing_sources})

set_target_properties (gspc_testing
  PROPERTIES C_VISIBILITY_PRESET hidden
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN ON
)

set_property (TARGET gspc_testing APPEND_STRING
  PROPERTY LINK_FLAGS " -rdynamic"
)

target_include_directories (gspc_testing
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_link_libraries (gspc_testing
  PUBLIC Boost::base
  PUBLIC Boost::program_options
  PUBLIC Boost::serialization
  PUBLIC Boost::unit_test_framework
  PUBLIC GPISpace-header-only
  PUBLIC gspc_util
  PUBLIC gspc_rpc
  PUBLIC gspc
  PUBLIC gspc_util
  PUBLIC fmt
)

if (GSPC_WITH_IML)
  target_link_libraries (gspc_testing PUBLIC gspc_iml_client)
endif()

target_compile_definitions (gspc_testing
  PUBLIC GSPC_SSL_CERTIFICATES_FOR_TESTS="${GSPC_SSL_CERTIFICATES_FOR_TESTS}"
  PUBLIC GSPC_WITH_IML=$<BOOL:${GSPC_WITH_IML}>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options (gspc_testing INTERFACE -fno-gnu-unique)
endif()

add_dependencies (gspc_testing test-certificates)

# Header-only interface for tests that can't link the full gspc_testing
# library (e.g., tests that must not have gspc_api_guard symbols
# available)
add_library (gspc_testing-header-only INTERFACE)
add_library (gspc_testing::header-only ALIAS gspc_testing-header-only)
target_include_directories (gspc_testing-header-only
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_link_libraries (gspc_testing-header-only
  INTERFACE Boost::unit_test_framework
)

# Qt testing utilities
find_package (Qt5 COMPONENTS Core Gui Widgets QUIET)
if (Qt5_FOUND)
  add_library (gspc_testing_util_qt STATIC
    "util/qt/Application.cpp"
    "util/qt/CoreApplication.cpp"
    "util/qt/detail/FakeCommandLineArguments.cpp"
    "util/qt/detail/MinimalPlatformInitializer.cpp"
  )
  target_include_directories (gspc_testing_util_qt
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
  target_link_libraries (gspc_testing_util_qt
    PUBLIC gspc_util_qt
    PUBLIC Qt5::Core
    PUBLIC Qt5::Gui
    PUBLIC Qt5::Widgets
  )
endif()
