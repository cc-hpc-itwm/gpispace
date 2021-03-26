# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
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

find_package (Libssh2 REQUIRED COMPONENTS _relocatable)

fhg_add_runtime_executable (NAME gspc-rifd
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES RPC
            fhg-util
            gspc::logging
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
  SOURCES "strategy/local.cpp"
          "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/ssh.cpp"
          "strategy/ssh/context.cpp"
          "strategy/ssh/detail.cpp"
          "strategy/ssh/session.cpp"
  LIBRARIES RPC
            fhg-util
            Libssh2::libssh2
            OpenSSL::Crypto
)

fhg_add_runtime_executable (NAME gspc-bootstrap-rifd
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-project_configuration
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
            fhg-project_configuration
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
