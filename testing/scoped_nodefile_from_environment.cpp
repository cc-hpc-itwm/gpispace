// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <testing/scoped_nodefile_from_environment.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>

#include <util-generic/getenv.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>

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
    ::boost::optional<const char*> const gspc_nodefile_for_tests
      (fhg::util::getenv ("GSPC_NODEFILE_FOR_TESTS"));
    if (!gspc_nodefile_for_tests)
    {
      throw std::runtime_error
        ("Environment variable GSPC_NODEFILE_FOR_TESTS is not set");
    }

    ::boost::filesystem::path const gspc_nodefile_for_tests_path
      (*gspc_nodefile_for_tests);

    if (!::boost::filesystem::exists (gspc_nodefile_for_tests_path))
    {
      throw std::runtime_error
        (( ::boost::format ("Environment variable GSPC_NODEFILE_FOR_TESTS=\"%1%\" points to invalid location.")
         % gspc_nodefile_for_tests_path
         ).str()
        );
    }

    ::boost::filesystem::copy_file (gspc_nodefile_for_tests_path, _temporary_file);

    gspc::set_nodefile (vm, _temporary_file);
  }
}
