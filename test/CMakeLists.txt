# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

set (_default_test_certs "${CMAKE_CURRENT_BINARY_DIR}/certs")
set (GSPC_SSL_CERTIFICATES_FOR_TESTS "${_default_test_certs}"
  CACHE STRING "SSL certificates to use in tests"
)

message (STATUS "Using '${GSPC_SSL_CERTIFICATES_FOR_TESTS}' for tests using SSL")

set (_test_rsa_bits 1024)
set (_test_dh_bits 1024)

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

configure_file ("make_environment.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/make_environment.hpp"
  @ONLY ESCAPE_QUOTES
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
                      PRIVATE "${CMAKE_BINARY_DIR}"
  LIBRARIES fhg-util
            Boost::program_options
            Boost::filesystem
            Util::Generic
            RPC
  COMPILE_DEFINITIONS GSPC_SSL_CERTIFICATES_FOR_TESTS="${GSPC_SSL_CERTIFICATES_FOR_TESTS}"
  DEPENDS test-certificates
)

fhg_add_test (NAME many_ranges_in_memory_transfer
  SOURCES "many_ranges_in_memory_transfer/driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/many_ranges_in_memory_transfer"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --size-of-range 1000
       --distance-between-ranges 24
       --number-of-ranges 16384
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

fhg_add_test (NAME many_ranges_in_memory_transfer_minimal
  SOURCES "many_ranges_in_memory_transfer/driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/many_ranges_in_memory_transfer"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --size-of-range 1
       --distance-between-ranges 0
       --number-of-ranges 16384
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
