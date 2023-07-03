// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <fhg/project_version.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/program_options.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_without_version)
{
  fhg::util::temporary_path const temporary_path;
  ::boost::filesystem::path const path (temporary_path);

  fhg::util::testing::require_exception
    ( [&path]()
      {
        ::boost::program_options::variables_map vm;
        gspc::set_gspc_home (vm, path);
        gspc::installation const installation (vm);
      }
    , std::invalid_argument
        ( ( ::boost::format ("GSPC version mismatch: File '%1%' does not exist.")
          % (::boost::filesystem::canonical (path) / "version")
          ).str()
        )
    );
}

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_with_bad_version)
{
  fhg::util::temporary_path const temporary_path;
  ::boost::filesystem::path const path (temporary_path);

  std::ofstream ((path / "version").string()) << "-";

  fhg::util::testing::require_exception
    ( [&path]()
      {
        ::boost::program_options::variables_map vm;
        gspc::set_gspc_home (vm, path);
        gspc::installation const installation (vm);
      }
    , std::invalid_argument
        ( ( ::boost::format ( "GSPC version mismatch: Expected '%1%'"
                            ", installation in '%2%' has version '%3%'"
                          )
          % fhg::project_version()
          % ::boost::filesystem::canonical (path)
          % "-"
          ).str()
        )
    );
}
