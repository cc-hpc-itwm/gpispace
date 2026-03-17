// Copyright (C) 2015-2016,2020-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/drts.hpp>

#include <gspc/configuration/version.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/program_options.hpp>

#include <fmt/core.h>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fstream>

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_without_version)
{
  gspc::testing::temporary_path const temporary_path;
  std::filesystem::path const path (temporary_path);

  gspc::testing::require_exception
    ( [&path]()
      {
        ::boost::program_options::variables_map vm;
        gspc::set_gspc_home (vm, path);
        gspc::installation const installation (vm);
      }
    , std::invalid_argument
      { fmt::format
        ( "GSPC version mismatch: File '{}' does not exist."
        , std::filesystem::canonical (path) / "gspc_version"
        )
      }
    );
}

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_with_bad_version)
{
  gspc::testing::temporary_path const temporary_path;
  std::filesystem::path const path (temporary_path);

  std::ofstream ((path / "gspc_version").string()) << "-";

  gspc::testing::require_exception
    ( [&path]()
      {
        ::boost::program_options::variables_map vm;
        gspc::set_gspc_home (vm, path);
        gspc::installation const installation (vm);
      }
    , std::invalid_argument
        { fmt::format
          ( "GSPC version mismatch: Expected '{}'"
            ", installation in '{}' has version '{}'"
          , gspc::configuration::version()
          , std::filesystem::canonical (path)
          , "-"
          )
        }
    );
}
