// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <iml/RuntimeSystem.hpp>

#include <boost/filesystem/operations.hpp>

namespace iml_test
{
  void set_iml_vmem_socket_path_for_localhost
    (::boost::program_options::variables_map& vm)
  {
    iml::RuntimeSystem::set_socket
      ( vm
      , ::boost::filesystem::temp_directory_path()
      / ::boost::filesystem::unique_path()
      );
  }
}
