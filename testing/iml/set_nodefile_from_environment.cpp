// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/set_nodefile_from_environment.hpp>

#include <gspc/iml/Rifs.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace iml_test
{
  std::filesystem::path set_nodefile_from_environment
    (::boost::program_options::variables_map& vm)
  {
    auto nodefile_for_tests {std::getenv ("GSPC_NODEFILE_FOR_TESTS")};

    if (!nodefile_for_tests)
    {
      throw std::runtime_error
        ( "Environment variable GSPC_NODEFILE_FOR_TESTS is not set"
        );
    }

    if (!std::filesystem::exists (nodefile_for_tests))
    {
      throw std::invalid_argument
        { fmt::format
            ( "Environment variable GSPC_NODEFILE_FOR_TESTS=\"{}\""
              " points to non-existent path."
            , nodefile_for_tests
            )
        };
    }

    gspc::iml::Rifs::set_nodefile (vm, nodefile_for_tests);

    return nodefile_for_tests;
  }
}
