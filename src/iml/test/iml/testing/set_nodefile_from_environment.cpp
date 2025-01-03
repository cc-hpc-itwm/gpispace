// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/set_nodefile_from_environment.hpp>

#include <iml/Rifs.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <fmt/core.h>
#include <cstdlib>
#include <stdexcept>

namespace iml_test
{
  ::boost::filesystem::path set_nodefile_from_environment
    (::boost::program_options::variables_map& vm)
  {
    auto iml_nodefile_for_tests (std::getenv ("IML_NODEFILE_FOR_TESTS"));

    // \todo Remove automatic fallback once standalone. Convenience
    // for transition period only. Once moved out this automatic
    // setting should be done on the GPISpace-side test level: A
    // test that uses IML shall set that variable.
    if (!iml_nodefile_for_tests)
    {
      iml_nodefile_for_tests = std::getenv ("GSPC_NODEFILE_FOR_TESTS");
    }

    if (!iml_nodefile_for_tests)
    {
      throw std::runtime_error
        ("Environment variable IML_NODEFILE_FOR_TESTS is not set");
    }

    if (!::boost::filesystem::exists (iml_nodefile_for_tests))
    {
      throw std::invalid_argument
        { fmt::format
            ( "Environment variable IML_NODEFILE_FOR_TESTS=\"{}\" "
              "points to non-existent path."
            , iml_nodefile_for_tests
            )
        };
    }

    iml::Rifs::set_nodefile (vm, iml_nodefile_for_tests);

    return iml_nodefile_for_tests;
  }
}
