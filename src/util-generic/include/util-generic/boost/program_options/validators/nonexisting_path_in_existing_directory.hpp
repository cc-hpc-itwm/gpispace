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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP

#include <util-generic/boost/program_options/validators.hpp>
#include <util-generic/boost/program_options/validators/existing_directory.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct nonexisting_path_in_existing_directory : public nonexisting_path
        {
          nonexisting_path_in_existing_directory (std::string const& option)
            : nonexisting_path (option)
            , _existing_directory
                (::boost::filesystem::path (option).parent_path().string())
          {}

          nonexisting_path_in_existing_directory
              (::boost::filesystem::path const& p)
            : nonexisting_path_in_existing_directory (p.string())
          {}

        private:
          existing_directory _existing_directory;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonexisting_path_in_existing_directory*
                             , int
                             )
        {
          validate<nonexisting_path_in_existing_directory> (v, values);
        }
      }
    }
  }
}

#endif
