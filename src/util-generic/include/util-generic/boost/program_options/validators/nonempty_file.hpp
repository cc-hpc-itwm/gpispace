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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_FILE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_FILE_HPP

#include <util-generic/boost/program_options/validators.hpp>

#include <util-generic/boost/program_options/validators/existing_path.hpp>

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
        struct nonempty_file : public existing_path
        {
          nonempty_file (std::string const& option)
            : existing_path (option)
          {
            if (! (::boost::filesystem::file_size (*this) > 0))
            {
              throw ::boost::program_options::invalid_option_value
                ((::boost::format ("%1% has size zero.") % *this).str());
            }
          }

          nonempty_file (::boost::filesystem::path const& p)
            : nonempty_file (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonempty_file*
                             , int
                             )
        {
          validate<nonempty_file> (v, values);
        }
      }
    }
  }
}

#endif
