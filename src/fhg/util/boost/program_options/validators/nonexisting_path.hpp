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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP

#include <fhg/util/boost/program_options/validators.hpp>

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
        struct nonexisting_path : public boost::filesystem::path
        {
          nonexisting_path (std::string const& option)
            : boost::filesystem::path (option)
          {
            if (boost::filesystem::exists (*this))
            {
              throw boost::program_options::invalid_option_value
                ((boost::format ("%1% already exists.") % *this).str());
            }
          }

          nonexisting_path (boost::filesystem::path const& p)
            : nonexisting_path (p.string())
          {}
        };

        inline void validate ( boost::any& v
                             , const std::vector<std::string>& values
                             , nonexisting_path*
                             , int
                             )
        {
          validate<nonexisting_path> (v, values);
        }
      }
    }
  }
}

#endif
