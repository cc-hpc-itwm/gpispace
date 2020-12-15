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

fhg_add_runtime_executable (NAME gpi-space
  SOURCES "gpi-space.cpp"
  LIBRARIES gpi-space-gpiapi
            gpi-space-pc-container
            gpi-space-pc-global
            gpi-space-pc-memory
            gpi-space-pc-segment
            fhg-project_configuration
            fhg-util
            mmgr
            fhgcom
            rif-started_process_promise
            Util::Generic
            ${CMAKE_THREAD_LIBS_INIT}
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

fhg_add_runtime_executable (NAME pnet2dot
  SOURCES "pnet2dot.cpp"
  LIBRARIES fhg-project_configuration
            pnet
            Util::Generic
            Boost::program_options
            Boost::serialization
            Boost::system
)

fhg_add_runtime_executable (NAME pnetc
  SOURCES "pnetc.cpp"
  LIBRARIES fhg-project_configuration
            pnet
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
)

fhg_add_runtime_executable (NAME agent
  SOURCES "agent.cpp"
  LIBRARIES sdpa
            pnet
            Util::Generic
            rif-started_process_promise
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)
