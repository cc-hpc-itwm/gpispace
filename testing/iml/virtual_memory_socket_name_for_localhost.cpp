// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/iml/RuntimeSystem.hpp>

#include <gspc/testing/unique_path.hpp>

#include <filesystem>

namespace iml_test
{
  void set_iml_vmem_socket_path_for_localhost
    (::boost::program_options::variables_map& vm)
  {
    gspc::iml::RuntimeSystem::set_socket
      ( vm
      , std::filesystem::temp_directory_path()
      / gspc::testing::unique_path()
      );
  }
}
