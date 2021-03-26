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

fhg_add_runtime_executable (NAME drts-kernel
  SOURCES "drts-kernel.cpp"
          "drts.cpp"
  LIBRARIES fhg-util
            fhgcom
            sdpa
            we-dev
            IML::Client
            rif-started_process_promise
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
            Boost::thread
            HWLOC::static
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

install (FILES context_fwd.hpp context.hpp
  DESTINATION include/drts/worker
)

extended_add_library (NAME drts-context
  SOURCES "context.cpp"
  LIBRARIES gspc::logging
            Boost::iostreams
  TYPE SHARED
  INSTALL
  CREATE_BUNDLE_INFO
)
