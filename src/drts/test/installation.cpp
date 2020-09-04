// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <fhg/revision.hpp>

#include <boost/program_options.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_without_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  fhg::util::testing::require_exception
    ([&path]()
    {
      boost::program_options::variables_map vm;
      gspc::set_gspc_home (vm, path);
      gspc::installation const installation (vm);
    }
    , std::invalid_argument
        ( ( boost::format ("GSPC revision mismatch: File '%1%' does not exist.")
          % (boost::filesystem::canonical (path) / "revision")
          ).str()
        )
    );
}

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_with_bad_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  std::ofstream ((path / "revision").string()) << "-";

  fhg::util::testing::require_exception
    ([&path]()
    {
      boost::program_options::variables_map vm;
      gspc::set_gspc_home (vm, path);
      gspc::installation const installation (vm);
    }
    , std::invalid_argument
        ( ( boost::format ( "GSPC revision mismatch: Expected '%1%'"
                            ", installation in '%2%' has version '%3%'"
                          )
          % fhg::project_revision()
          % boost::filesystem::canonical (path)
          % "-"
          ).str()
        )
    );
}
