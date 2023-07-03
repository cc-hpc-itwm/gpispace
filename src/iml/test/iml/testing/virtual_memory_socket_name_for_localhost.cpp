// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
