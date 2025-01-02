// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/scoped_nodefile_from_environment.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>

#include <boost/optional.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <cstdlib>
#include <fmt/core.h>
#include <fstream>
#include <stdexcept>

namespace test
{
  scoped_nodefile_from_environment::scoped_nodefile_from_environment
    ( ::boost::filesystem::path const& shared_directory
    , ::boost::program_options::variables_map& vm
    )
      : _temporary_file (shared_directory / ::boost::filesystem::unique_path())
  {
    auto const gspc_nodefile_for_tests
      {std::getenv ("GSPC_NODEFILE_FOR_TESTS")};
    if (!gspc_nodefile_for_tests)
    {
      throw std::runtime_error
        ("Environment variable GSPC_NODEFILE_FOR_TESTS is not set");
    }

    ::boost::filesystem::path const gspc_nodefile_for_tests_path
      (gspc_nodefile_for_tests);

    if (!::boost::filesystem::exists (gspc_nodefile_for_tests_path))
    {
      throw std::runtime_error
        { fmt::format
            ( "Environment variable GSPC_NODEFILE_FOR_TESTS=\"{}\" points to invalid location."
            , gspc_nodefile_for_tests_path
            )
        };
    }

    ::boost::filesystem::copy_file (gspc_nodefile_for_tests_path, _temporary_file);

    gspc::set_nodefile (vm, _temporary_file);
  }
}
