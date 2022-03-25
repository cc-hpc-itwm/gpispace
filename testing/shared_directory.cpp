// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <testing/shared_directory.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>

namespace test
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      namespace
      {
        constexpr char const* const shared_directory {"shared-directory"};
      }
    }

    ::boost::program_options::options_description shared_directory()
    {
      ::boost::program_options::options_description shared_directory;

      shared_directory.add_options()
        ( name::shared_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "directory shared between workers"
        )
        ;

      return shared_directory;
    }
  }

  ::boost::filesystem::path shared_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::shared_directory)
      .as<validators::existing_directory>();
  }
}
