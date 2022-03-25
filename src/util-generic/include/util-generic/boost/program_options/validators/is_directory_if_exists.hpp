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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP

#include <util-generic/boost/program_options/validators.hpp>

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
        struct is_directory_if_exists : public ::boost::filesystem::path
        {
          is_directory_if_exists (std::string const& option)
            : ::boost::filesystem::path (option)
          {
            if ( ::boost::filesystem::exists (*this)
               && !::boost::filesystem::is_directory (*this)
               )
            {
              throw ::boost::program_options::invalid_option_value
                (( ::boost::format ("%1% exists and is not a directory.")
                 % *this
                 ).str()
                );
            }
          }

          is_directory_if_exists (::boost::filesystem::path const& p)
            : is_directory_if_exists (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , is_directory_if_exists*
                             , int
                             )
        {
          validate<is_directory_if_exists> (v, values);
        }
      }
    }
  }
}

#endif
