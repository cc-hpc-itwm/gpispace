// Copyright (C) 2014,2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/scoped_nodefile_from_environment.hpp>

#include <gspc/drts/drts.hpp>
#include <gspc/drts/private/option.hpp>

#include <gspc/testing/unique_path.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <cstdlib>
#include <fmt/core.h>
#include <fstream>
#include <stdexcept>

namespace gspc::testing
{
  scoped_nodefile_from_environment::scoped_nodefile_from_environment
    ( std::filesystem::path const& shared_directory
    , ::boost::program_options::variables_map& vm
    )
      : _temporary_file
        ( shared_directory / gspc::testing::unique_path()
        )
  {
    auto const gspc_nodefile_for_tests
      {std::getenv ("GSPC_NODEFILE_FOR_TESTS")};
    if (!gspc_nodefile_for_tests)
    {
      throw std::runtime_error
        ("Environment variable GSPC_NODEFILE_FOR_TESTS is not set");
    }

    std::filesystem::path const gspc_nodefile_for_tests_path
      (gspc_nodefile_for_tests);

    if (!std::filesystem::exists (gspc_nodefile_for_tests_path))
    {
      throw std::runtime_error
        { fmt::format
            ( "Environment variable GSPC_NODEFILE_FOR_TESTS=\"{}\" points to invalid location."
            , gspc_nodefile_for_tests_path
            )
        };
    }

    std::filesystem::copy_file (gspc_nodefile_for_tests_path, _temporary_file);

    gspc::set_nodefile (vm, std::filesystem::path {_temporary_file});
  }
}
