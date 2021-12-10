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

#include <iml/testing/beegfs_directory.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>

namespace iml_test
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      namespace
      {
        constexpr char const* const beegfs_directory {"beegfs-directory"};
      }
    }

    ::boost::program_options::options_description beegfs_directory()
    {
      ::boost::program_options::options_description beegfs_directory;

      beegfs_directory.add_options()
        ( name::beegfs_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "BeeGFS directory to store test data in"
        )
        ;

      return beegfs_directory;
    }
  }

  ::boost::filesystem::path beegfs_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::beegfs_directory)
      .as<validators::existing_directory>();
  }
}
