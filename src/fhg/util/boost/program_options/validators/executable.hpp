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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP

#include <fhg/util/boost/program_options/validators.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <system_error>

#include <unistd.h>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct executable : public existing_path
        {
          executable (std::string const& option)
            : existing_path (option)
          {
            int const rc (::access (c_str(), F_OK | X_OK));
            int const errno_value (errno);
            if (rc == -1)
            {
              throw boost::program_options::invalid_option_value
                ( ( boost::format ("%1% is not executable: %2%")
                  % *this
                  % std::system_category().message (errno_value)
                  ).str()
                );
            }
          }

          executable (boost::filesystem::path const& path)
            : executable (path.string())
          {}
        };

        inline void validate ( boost::any& v
                             , const std::vector<std::string>& values
                             , executable*
                             , int
                             )
        {
          validate<executable> (v, values);
        }
      }
    }
  }
}

#endif
