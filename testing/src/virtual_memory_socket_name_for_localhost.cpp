// Copyright (C) 2014,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/drts/private/option.hpp>

#include <gspc/testing/unique_path.hpp>
#include <filesystem>

namespace gspc::testing
{
  void set_virtual_memory_socket_name_for_localhost
    (::boost::program_options::variables_map& vm)
  {
    gspc::set_virtual_memory_socket ( vm
                                    , std::filesystem::temp_directory_path()
                                    / gspc::testing::unique_path()
                                    );
  }
}
